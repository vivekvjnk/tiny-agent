# [Generic Interrupt Controller (GIC)](https://support.arm.com/documentation/198123/0302/What-is-a-Generic-Interrupt-Controller-)

# ARM Generic Interrupt Controller (GIC): Low-Level Firmware Reference

From an embedded driver and silicon bringup perspective, the **Generic Interrupt Controller (GIC)** is a hardware peripheral mapped into memory (MMIO) or accessed via CPU system registers. It arbitrates, prioritizes, and routes physical and virtual interrupts across single- or multi-core ARM systems.

---

## 1. Hardware Architecture & Topology

The GIC logic is partitioned into three functional blocks:

```
+-----------------------------------------------------------------------------------+
|                                 GIC SUBSYSTEM                                     |
|                                                                                   |
|  +--------------------+       +---------------------+       +------------------+  |
|  |    Distributor     |       |    Redistributor    |       |   CPU Interface  |  |
|  |      (GICD)        | ----> |       (GICR)        | ----> |  (GICC / System) |  |
|  +--------------------+       +---------------------+       +------------------+  |
|            ^                             ^                            |           |
+------------|-----------------------------|----------------------------|-----------+
             |                             |                            v
    +-----------------+           +-----------------+           +---------------+
    | Shared Periphs  |           | Private Periphs |           | CPU Core / PE |
    |   (SPI Lines)   |           | (PPI Timers/PMU)|           | (IRQ / FIQ)   |
    +-----------------+           +-----------------+           +---------------+

```

* 
**Distributor (`GICD`):** Global controller. Manages global interrupt settings, priority sorting, and routing of **Shared Peripheral Interrupts (SPIs)** to target cores.


* **Redistributor (`GICR`):** Per-core interface in GICv3/v4. Handles core-private interrupts (**PPIs**, **SGIs**) and message-signaled interrupts (**LPIs**).


* **CPU Interface (`GICC` / System Registers):** Interfaces directly with the processing element (PE). Handles priority masking, interrupt acknowledgment (`IAR`), and End of Interrupt (`EOIR`) signalling.

---

## 2. Interrupt Types & Allocation Mapping

For driver and Device Tree (`DTS`) configuration, interrupts are classified into four distinct types:

| Type | Name | ID Range | Scope | Trigger Type | Common Firmware Use Case |
| --- | --- | --- | --- | --- | --- |
| **SGI** | Software Generated Interrupt | `0 - 15` | Core-to-Core | Software write | Inter-Processor Communication (IPI / SMP resched) 

 |
| **PPI** | Private Peripheral Interrupt | `16 - 31` | Per-Core Local | Edge / Level | Architectural Timers, PMUs, Local Watchdogs 

 |
| **SPI** | Shared Peripheral Interrupt | `32 - 1019` | System Wide | Edge / Level | UART, DMA Controllers, PCIe Endpoints, I2C 

 |
| **LPI** | Locality-specific Peripheral | `8192+` | System Wide | Message-Signaled | PCIe MSI / MSI-X via Interrupt Translation Service (ITS) 

 |

---

## 3. The Hardware Interrupt State Machine

Each interrupt line managed by the GIC transitions through four distinct states during execution:

```
                  +--------------------------------+
                  |            INACTIVE            |
                  +--------------------------------+
                     |                          ^
     Interrupt Line  |                          | End of Interrupt
     Asserted        v                          | (EOIR Write)
                  +--------------------------------+
                  |            PENDING             |
                  +--------------------------------+
                     |                          ^
     CPU Acknowledges|                          | Interrupt Re-asserted
     (IAR Read)      v                          | before EOI
                  +--------------------------------+
                  |             ACTIVE             |
                  +--------------------------------+
                     |                          ^
                     |   ACTIVE & PENDING       |
                     +--------------------------+

```

1. **Inactive:** The interrupt source is silent.
2. **Pending:** Hardware line asserted or SGI generated, but not yet acknowledged by a core.
3. **Active:** CPU has acknowledged the interrupt (read `IAR`) and is currently running the Interrupt Service Routine (ISR).
4. **Active & Pending:** CPU is executing the ISR, but the same interrupt source has re-asserted a new signal.

---

## 4. Key Register Programming Model

During board bringup and driver initialization, firmware interacts with key memory-mapped or CPU system registers:

### Distributor & Core Configuration Registers

| Register Name | Type | Description / Usage |
| --- | --- | --- |
| **`GICD_CTLR`** | MMIO | Global enable/disable for Secure and Non-Secure Groups. |
| **`GICD_ISENABLERn`** | MMIO | Set-Enable register bits for enabling individual IRQ IDs. |
| **`GICD_ICENABLERn`** | MMIO | Clear-Enable register bits for disabling IRQ IDs. |
| **`GICD_IPRIORITYRn`** | MMIO | Sets 8-bit priority per interrupt (lower numerical value = higher priority). |
| **`GICD_IROUTERn`** | MMIO | Defines target CPU affinity/core routing for a specific SPI. |

### CPU Interface Registers (GICv3 System Registers)

| System Register | Access | Description / Firmware Execution Flow |
| --- | --- | --- |
| **`ICC_PMR_EL1`** | `msr/mrs` | Priority Mask Register. Drops interrupts with lower priority than configured mask. |
| **`ICC_IAR1_EL1`** | `mrs` | **Interrupt Acknowledge:** Read in Top-Half ISR to retrieve the active Interrupt ID. |
| **`ICC_EOIR1_EL1`** | `msr` | **End of Interrupt:** Written at the end of handling to clear active state. |
| **`ICC_IGRPEN1_EL1`** | `msr` | Group 1 Interrupt Enable bit for Non-Secure execution. |

---

## 5. Low-Level Firmware Driver Handling Flow

When an interrupt fires, the CPU execution sequence follows this pattern:

```
[Hardware Assert] ---> [GIC Arbitrates Priority] ---> [Assert Core IRQ Line]
                                                                |
                                                                v
[Clear Active State] <-- [Write ICC_EOIR1_EL1] <-- [Run Subsystem ISR] <-- [Read ICC_IAR1_EL1]

```

### C Driver Top-Half Handler Implementation

```c
#include <stdint.h>

// Assembly helper wrappers for ARM64 System Registers
static inline uint64_t read_icc_iar1_el1(void) {
    uint64_t iar;
    asm volatile("mrs %0, ICC_IAR1_EL1" : "=r" (iar));
    return iar;
}

static inline void write_icc_eoir1_el1(uint64_t eoir) {
    asm volatile("msr ICC_IAR1_EL1, %0" :: "r" (eoir));
    asm volatile("isb"); // Instruction Synchronization Barrier
}

// Low-Level IRQ Vector Handler (Called directly from exception vector)
void gic_v3_irq_handler(void) {
    uint64_t iar = read_icc_iar1_el1();
    uint32_t irq_id = (uint32_t)(iar & 0xFFFFFF); // Extract 24-bit INTID

    // Check for Spurious Interrupt ID (1023)
    if (irq_id >= 1020) {
        return;
    }

    // 1. Dispatch handler registered for this specific INTID
    dispatch_peripheral_isr(irq_id);

    // 2. Complete interrupt cycle in GIC hardware
    write_icc_eoir1_el1(iar);
}

```

---

## Summary Checklist for Silicon & Driver Bringup

* [ ] **Device Tree Binding:** Confirm SPI vs PPI interrupt index offset (Device tree SPI index typically starts at offset `0`, which maps to physical INTID `32` + `index`).


* [ ] **Affinity Routing:** Set `GICD_IROUTERn` to target specific core affinities (`MPIDR_EL1`) for multi-core distribution.
* [ ] **Memory Barrier Execution:** Always issue a `DSB` (Data Synchronization Barrier) before writing to doorbell registers, and an `ISB` after writing to `ICC_*_EL1` CPU interface registers to enforce memory access order.


