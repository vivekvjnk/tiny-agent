# Arm Timer Architectures: Generic Timer (Cortex-A/R) & SysTick (Cortex-M)

Arm processors utilize standardized hardware timer subsystems to deliver deterministic ticks, timekeeping, delays, and context switching across bare-metal, RTOS, and Linux environments.

---

## 1. Subsystem Architectural Comparison

| Feature / Metric | SysTick Timer | Arm Generic Timer |
| --- | --- | --- |
| **Target Architecture** | Armv6-M, Armv7-M, Armv8-M (Cortex-M) | Armv7-A/R, Armv8-A/R, Armv9-A (Cortex-A / Cortex-R) |
| **Counter Direction** | 24-bit Down-Counter | 64-bit Free-Running Up-Counter |
| **System Scope** | Local per-core timer | System-wide global counter with per-core virtual/physical timers |
| **Frequency Stability** | Dependent on CPU clock / core sleep | Fixed frequency system crystal (unaffected by CPU DVFS / sleep) |
| **Virtualization Support** | None | Native EL2 hypervisor and EL1 virtual timer offsets (`CNTVOFF_EL2`) |
| **Programming Interface** | MMIO registers (`0xE000E010`) | System registers (`mrs` / `msr` instructions) |

---

## 2. Generic Timer Subsystem Topology

```
+---------------------------------------------------------------------------------+
| GLOBAL SYSTEM COUNTER (Always-On Power Domain)                                  |
|                                                                                 |
|   • Generates global 64-bit tick count (e.g. 19.2 MHz / 54 MHz base clock)     |
|   • Programmed via System Counter Control Registers (CNTCR / CNTFID0)           |
+---------------------------------------------------------------------------------+
                                         │
                   Distributes System Count Bus (CNTCV)
                                         │
       ┌─────────────────────────────────┴─────────────────────────────────┐
       ▼                                                                   ▼
+-----------------------------------+             +-----------------------------------+
| CORE 0 ARCHITECTURAL TIMER        |             | CORE 1 ARCHITECTURAL TIMER        |
|                                   |             |                                   |
|  • EL1 Physical Timer (CNTP_*)    |             |  • EL1 Physical Timer (CNTP_*)    |
|  • EL1 Virtual Timer  (CNTV_*)    |             |  • EL1 Virtual Timer  (CNTV_*)    |
|  • EL2 Hypervisor Timer (CNTH_*)  |             |  • EL2 Hypervisor Timer (CNTH_*)  |
|  • EL3 Secure Timer   (CNTS_*)    |             |  • EL3 Secure Timer   (CNTS_*)    |
+-----------------------------------+             +-----------------------------------+
```

---

## 3. Key Register Map & Bitfield Reference

In AArch64, timers are accessed via CPU System Registers (`mrs` / `msr` instructions):

| Register | Access Scope | Description / Functional Operation |
| --- | --- | --- |
| **`CNTFRQ_EL0`** | Read / Write (EL3) | **Counter Frequency Register:** Stores system counter frequency in Hz (e.g., `0x0124F800` = 19.2 MHz). Initialized by TF-A in EL3. |
| **`CNTPCT_EL0`** | Read-Only | **Physical Count Register:** Returns current 64-bit raw physical counter value. |
| **`CNTVCT_EL0`** | Read-Only | **Virtual Count Register:** Returns physical count minus virtual offset (`CNTPCT_EL0 - CNTVOFF_EL2`). |
| **`CNTP_CVAL_EL0`** | Read / Write | **Physical Timer Compare Value Register:** Interrupt fires when `CNTPCT_EL0 >= CNTP_CVAL_EL0`. |
| **`CNTP_TVAL_EL0`** | Read / Write | **Physical Timer Timer Value Register:** 32-bit down-counter timer reload value. Automatically sets `CNTP_CVAL_EL0 = CNTPCT_EL0 + TVAL`. |
| **`CNTP_CTL_EL0`** | Read / Write | **Physical Timer Control Register:** Bit `0` (`ENABLE`), Bit `1` (`IMASK` interrupt mask), Bit `2` (`ISTATUS` interrupt status). |

---

## 4. Interrupt Assignment Matrix (PPI Allocation)

Generic Timers generate Private Peripheral Interrupts (PPIs) routed to each CPU core's GIC Redistributor:

| Timer Channel | INTID Range | Trigger Type | Common OS / Firmware Usage |
| --- | --- | --- | --- |
| **Secure Physical Timer** | PPI 29 (INTID 29) | Level Active-Low | Secure OS / TF-A runtime events |
| **Non-Secure Physical Timer** | PPI 30 (INTID 30) | Level Active-Low | Linux Kernel primary `clock_event_device` tick |
| **Virtual Timer** | PPI 27 (INTID 27) | Level Active-Low | Guest OS tick running inside KVM virtual machine |
| **Hypervisor Physical Timer** | PPI 26 (INTID 26) | Level Active-Low | KVM Hypervisor scheduler / timer management |

---

## 5. Linux Kernel Driver Integration (`arm_arch_timer.c`)

### Device Tree Binding Reference

```dts
timer {
    compatible = "arm,armv8-timer";
    interrupts = <GIC_PPI 13 (GIC_CPU_MASK_SIMPLE(4) | IRQ_TYPE_LEVEL_LOW)>, /* Secure Physical */
                 <GIC_PPI 14 (GIC_CPU_MASK_SIMPLE(4) | IRQ_TYPE_LEVEL_LOW)>, /* Non-Secure Physical */
                 <GIC_PPI 11 (GIC_CPU_MASK_SIMPLE(4) | IRQ_TYPE_LEVEL_LOW)>, /* Virtual */
                 <GIC_PPI 10 (GIC_CPU_MASK_SIMPLE(4) | IRQ_TYPE_LEVEL_LOW)>; /* Hypervisor */
    clock-frequency = <19200000>; /* 19.2 MHz fallback if CNTFRQ_EL0 not set */
};
```

### Low-Level Register Timer Setup Code

```c
// Enable EL1 Physical Timer for 1ms delay (19.2MHz clock)
void arm_timer_set_1ms_tick(void) {
    uint64_t cntfrq;
    asm volatile("mrs %0, cntfrq_el0" : "=r" (cntfrq));

    // Program Timer Value register (TVAL = cntfrq / 1000)
    uint32_t tval = (uint32_t)(cntfrq / 1000);
    asm volatile("msr cntp_tval_el0, %0" :: "r" ((uint64_t)tval));

    // Enable timer and unmask interrupt (CTL = 1)
    uint64_t ctl = 1;
    asm volatile("msr cntp_ctl_el0, %0" :: "r" (ctl));
    asm volatile("isb");
}
```
