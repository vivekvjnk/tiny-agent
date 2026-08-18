# Master Architecture Hub: ARM Cortex-A SoC Boot & Silicon Bringup Pipeline

Welcome to the **SoC Boot Sequence & Hardware Bringup Architecture Hub**. This master document synthesizes the end-to-end initialization flow for Armv8/Armv9 Cortex-A Application Processors—from cold-power rail stabilization and hardware reset up to Linux kernel entry.

It integrates all core concepts, execution contexts, security models, hardware peripherals, and debugging procedures explored across the bringup curriculum.

---

## 1. System Initialization Architecture Map

```
 +---------------------------------------------------------------------------------------------------+
 | PHASE 0: HARDWARE POWER-ON RESET (POR) & PMIC STABILIZATION                                       |
 |   • PMIC Rail Sequence: VDD_CORE -> VDD_MEM -> VDD_IO                                             |
 |   • Clock Lock: Crystal Osc -> PLLs -> Release POR_N -> Sample Boot Pins                          |
 +---------------------------------------------------------------------------------------------------+
                                                   │
                                                   ▼
 +---------------------------------------------------------------------------------------------------+
 | PHASE 1: TRUSTED FIRMWARE-A (TF-A) EARLY BOOT & DDR INITIALIZATION                               |
 |   • [BL1 (Secure EL3)]: BootROM -> Loads & Authenticates BL2 into SRAM                           |
 |   • [BL2 (Secure EL1/EL3)]: DDR PHY Calibration (DQS Leveling, VREF) -> Loads BL31/BL32/BL33     |
 |     └─► Deep Dive: [BL2 Architecture & Memory Topology](BL2.md)                                  |
 +---------------------------------------------------------------------------------------------------+
                                                   │
                                                   ▼
 +---------------------------------------------------------------------------------------------------+
 | PHASE 2: SECURE EL3 RUNTIME & PRIVILEGE ARCHITECTURE                                              |
 |   • [BL31 (Secure EL3 Monitor)]: PSCI (CPU_ON, SUSPEND), SCMI (Clocks/DVFS), SMC Conventions         |
 |   • Security Isolation: SCR_EL3, TrustZone (TZASC/CSU), Exception Vector Tables (VBAR_ELx)       |
 |     ├─► Deep Dive: [Arm Trusted Firmware-A (TF-A) & PSCI/SCMI Services](TFA.md)                   |
 |     └─► Deep Dive: [ARM Exception Levels & Privilege Model](Exception-Levels-in-ARM.md)           |
 +---------------------------------------------------------------------------------------------------+
                                                   │
                                                   ▼
 +---------------------------------------------------------------------------------------------------+
 | PHASE 3: NON-SECURE BOOTLOADER EXECUTION (BL33)                                                   |
 |   • Execution Mode: Non-Secure EL2 (Hypervisor) / EL1                                             |
 |   • Software Target: U-Boot / Barebox (PBL -> VFS -> Bootspec)                                    |
 |   • Peripheral Drivers: Storage (eMMC/NVMe), Network (TFTP/NFS), USB Gadget                      |
 |     ├─► Deep Dive: [BL33 Non-Secure Bootloader Architecture](BL33.md)                              |
 |     └─► Deep Dive: [Barebox Architecture & Command Reference](../tools/Barebox.md)               |
 +---------------------------------------------------------------------------------------------------+
                                                   │
                                                   ▼
 +---------------------------------------------------------------------------------------------------+
 | PHASE 4: LOW-LEVEL HARDWARE INTEGRATION & CORE PERIPHERALS                                         |
 |   • Device Tree: Flattened DTB Layout (0xd00dfeed), Ranges Translation, Compatible Matching        |
 |   • Generic Interrupt Controller (GICv2/v3/v4): Distributor (GICD), Redistributor, CPU System Regs|
 |   • System Timers: Generic Timer (CNTPCT_EL0, CNTFRQ_EL0, PPIs) vs SysTick (Cortex-M)             |
 |     ├─► Deep Dive: [Device Tree Subsystem & Runtime Parsing](Device-Tree.md)                      |
 |     ├─► Deep Dive: [Generic Interrupt Controller (GIC) Low-Level Spec](Generic-Interrupt-Controller.md)
 |     └─► Deep Dive: [Arm Generic Timer Architecture](standard-arm-timer.md)                       |
 +---------------------------------------------------------------------------------------------------+
                                                   │
                                                   ▼
 +---------------------------------------------------------------------------------------------------+
 | PHASE 5: OS KERNEL DECOMPRESSION & ENTRY CONTRACT                                                 |
 |   • Decompression Mechanics: AArch32 zImage self-extraction vs AArch64 Image/ZBOOT (ZSTD/GZIP)   |
 |   • Entry Invariants (head.S): x0 = DTB Address, MMU = OFF, D-Cache = OFF, PSTATE Interrupts Masked|
 |     ├─► Deep Dive: [Linux Kernel Decompression Architecture](Kernel-Decompression.md)             |
 |     └─► Deep Dive: [Boot Troubleshooting & Root Cause Analysis Matrix](../questions/boot.md)     |
 +---------------------------------------------------------------------------------------------------+
```

---

## 2. Phase 0: Hardware Power-On Reset (POR) & Rail Sequencing

Before a single instruction is executed by Core 0, PMIC regulators and clock sources must execute a hardware power-ramp sequence to prevent parasitic CMOS latch-up and flash corruption:

```
Voltage / Signals
  ▲
  │
  ├─ VDD_CORE  ─────────────┌─────────────────────────────────────────────── (Core Logic Voltage)
  │                         │
  ├─ VDD_MEM   ─────────┌───┘                                                 (DRAM/SRAM Supply)
  │                     │
  ├─ VDD_IO    ─────┌───┘                                                     (Peripherals/I-O)
  │                 │
  ├─ SYS_CLK   ─────┼───────┌─┐─┌─┐─┌─┐─┌─┐─┌─┐─┌─┐─┌─┐─┌─┐─────────────────── (Crystal/PLL Lock)
  │                 │       │ │ │ │ │ │ │ │ │ │ │ │ │ │ │
  ├─ POR_N     ─────┴───────┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─└───┐───────────────── (Power-On Reset Released)
  │                                                         │
  └─────────────────────────────────────────────────────────┴─────────────────► Time
                                                            CPU Fetch Starts from BootROM
```

1. **Power Rail Ramp:** PMIC regulators ramp in hardwired sequence: $V_{\text{DD\_CORE}} \rightarrow V_{\text{DD\_MEM}} \rightarrow V_{\text{DD\_IO}}$.
2. **Clock Stabilization:** Primary crystal oscillator turns on $\rightarrow$ System PLLs lock $\rightarrow$ PMIC asserts Power-Good (`PG`).
3. **Reset De-assertion:** System controller releases `POR_N` (active low).
4. **Boot Pin Sampling:** Boot controller latches physical boot pin straps (eMMC vs SPI NOR vs PCIe Endpoint mode).
5. **BootROM Fetch:** Core 0 un-clears reset vector and fetches instruction 0 from internal BootROM SRAM.

---

## 3. Master Boot Stage Execution Matrix

| Stage | Software Entity | Execution Level | Memory Context | Core Responsibilities | Detailed Technical Reference |
| --- | --- | --- | --- | --- | --- |
| **BL1** | BootROM Firmware | Secure EL3 | On-Chip ROM / SRAM | Validates hardware straps, initializes SPI/eMMC boot storage, authenticates BL2 image via ROTPK. | [TF-A Architecture](TFA.md) |
| **BL2** | Trusted Boot Firmware | Secure EL1 / EL3 | Internal SRAM (128KB-512KB) | Configures PLL clock trees, PMIC voltage registers, executes **DDR PHY Calibration**, loads BL31/BL32/BL33 into DRAM. | [BL2 Summary](BL2.md) |
| **BL31** | TF-A Runtime Monitor | Secure EL3 | Secure SRAM / DRAM | Remains resident. Serves PSCI calls (`CPU_ON`), SCMI requests, manages SMC exceptions, handles Secure/Non-Secure world transitions. | [TF-A & PSCI/SCMI](TFA.md) |
| **BL32** | Secure Payload | Secure EL1 | Secure DRAM (TZASC) | Optional OP-TEE / Trusted OS for DRM, cryptographic keystore, and hardware isolation. | [Exception Levels](Exception-Levels-in-ARM.md) |
| **BL33** | Non-Secure Bootloader | Non-Secure EL2 / EL1 | Main DRAM | U-Boot or Barebox. Parses DTB, loads kernel image into DRAM, sets CPU registers for OS handoff. | [BL33 Architecture](BL33.md) & [Barebox Spec](../tools/Barebox.md) |
| **Kernel** | Linux Kernel (`head.S`) | Non-Secure EL1 / EL2 | Main DRAM | Decompresses payload (if required), sets up virtual page tables, enables MMU, probes DTB devices, initializes `init`. | [Kernel Decompression](Kernel-Decompression.md) & [Boot FAQ](../questions/boot.md) |

---

## 4. Hardware Core Subsystems Integration

During the BL33 and early kernel startup phases, three fundamental SoC hardware subsystems must be configured and bound via Device Tree:

### A. Device Tree Subsystem (`.dtb`)
* **Role:** Decouples board-specific hardware memory maps from the compiled Linux kernel image.
* **Key Mechanisms:** Flattened Device Tree header (`0xd00dfeed`), `#address-cells`/`#size-cells` bus address translation (`ranges`), and driver probing via `compatible` strings.
* **Full Specification:** See **[Device Tree Subsystem & Runtime Parsing](Device-Tree.md)**.

### B. Generic Interrupt Controller (GIC)
* **Role:** Arbitrates, prioritizes, and routes interrupts across multi-core ARM systems.
* **Key Components:** Distributor (`GICD`), Redistributor (`GICR`), and CPU Interface System Registers (`ICC_*_EL1`).
* **Interrupt Types:** SGIs (0-15), PPIs (16-31), SPIs (32-1019), LPIs (8192+).
* **Full Specification:** See **[Generic Interrupt Controller (GIC) Technical Reference](Generic-Interrupt-Controller.md)**.

### C. Standard Arm Generic Timers
* **Role:** Provides deterministic timekeeping, delays, and scheduler tick interrupts.
* **Key Components:** Global 64-bit free-running System Counter (`CNTCV`), CPU System Registers (`CNTFRQ_EL0`, `CNTPCT_EL0`, `CNTP_TVAL_EL0`), and core-private PPI interrupts (PPI 30 for Non-Secure Physical Timer).
* **Full Specification:** See **[Arm Generic Timer Architecture](standard-arm-timer.md)**.

---

## 5. Kernel Entry Contract (`head.S`) & Diagnostic Matrix

When BL33 hands off control to the Linux kernel, the CPU hardware context must satisfy this mandatory architectural contract:

| Register / Flag | Required Invariant State | Technical Reason / Failure Risk |
| --- | --- | --- |
| **`x0` Register** | **Physical memory address of `.dtb`** | Device tree address pointer. Invalid address triggers kernel panic during early memory setup. |
| **`x1`, `x2`, `x3` Regs** | **Must be `0`** | Reserved for future architecture specification extensions. |
| **MMU** | **Disabled** | Kernel virtual memory page tables are uninitialized. Enabled MMU triggers immediate Translation Fault. |
| **Data Cache (D-Cache)** | **Disabled** | Prevents stale physical memory cache hits before virtual paging and cache maintenance are initialized. |
| **Instruction Cache** | **Enabled or Disabled** | I-cache may be enabled for fast fetch; coherent with physical DRAM. |
| **PSTATE Interrupts** | **Masked (`.D`, `.A`, `.I`, `.F` = 1)** | All interrupts masked. Exception vector table (`VBAR_EL1`) is uninitialized. |
| **Execution Level** | **EL2 (Hypervisor) or EL1** | Booting at EL2 enables KVM hypervisor initialization; EL1 is standard OS boot. |

---

## 6. Central Index of Concept & Tool Modules

Below is the cross-reference index connecting all technical modules developed in this exploration:

1. **System Power & Runtime Services:**
   * [TF-A Services & PSCI / SCMI Protocol Specification](TFA.md)
   * [ARM Exception Levels, Security Worlds & Privilege Control](Exception-Levels-in-ARM.md)
2. **Boot Firmware & Bootloader Execution:**
   * [BL2 Trusted Boot Firmware & DDR PHY Training](BL2.md)
   * [BL33 Non-Secure Bootloader Architecture](BL33.md)
   * [Barebox Bootloader Architecture & Command Reference](../tools/Barebox.md)
3. **Core Peripherals & Hardware Tree:**
   * [Device Tree Architecture & Runtime Parsing](Device-Tree.md)
   * [Generic Interrupt Controller (GICv2/v3/v4) Technical Reference](Generic-Interrupt-Controller.md)
   * [Arm Generic Timer Subsystem Reference](standard-arm-timer.md)
4. **Kernel Mechanics & Troubleshooting:**
   * [Linux Kernel Decompression Architecture](Kernel-Decompression.md)
   * [Boot Sequence Troubleshooting & Root Cause Analysis FAQ](../questions/boot.md)
   * [7-Day SoC Exploration Plan](../exploration_plan.md)
