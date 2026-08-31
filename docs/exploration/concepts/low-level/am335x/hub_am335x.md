The Texas Instruments AM335x pairs an ARM Cortex-A8 core with industrial peripherals and dual real-time microcontrollers. The low-level hardware architecture breaks down across nine key domains:

**Cortex-A8 Core & L3/L4 Interconnect Fabric**

* **Core Architecture:** ARMv7-A 32-bit single-core, dual-issue superscalar pipeline running up to 1 GHz. Includes 32KB/32KB L1 instruction/data cache with parity and 256KB unified L2 cache with ECC.
* **Interconnect:** Uses the Crossbar Architecture Switched System (CBASS). The high-speed **L3 Matrix** (200 MHz) connects high-bandwidth masters (Cortex-A8, DMA, PRU-ICSS) to system memory (DDR, SRAM). The **L4 Fabric** (100 MHz/50 MHz) routes lower-bandwidth register traffic to peripherals (UART, SPI, Timers).

**NEON SIMD Engine**

* **Execution Pipeline:** 64/128-bit vector engine containing thirty-two 64-bit vector registers.
* **Low-Level Detail:** Shares the execution pipeline with the VFPv3 floating-point unit. Vector operations run through a dedicated sub-pipeline with additional register-transfer latency; compilers require careful instruction scheduling to prevent pipeline stalls when moving values back to ARM integer registers.

**Interrupt Controller (AINTC)**

* **Architecture:** Custom TI ARM Interrupt Controller (AINTC), **not** a standard ARM GIC.
* **Hardware Capabilities:** Supports up to 128 hardware interrupt lines mapped across 128 priority levels. Features hardware auto-prioritization and automatic vector generation (`SIR_IRQ` / `SIR_FIQ` registers) to eliminate software vector lookup overhead in critical ISRs.

**General Purpose Memory Controller (GPMC) & Error Locator Module (ELM)**

* **GPMC:** Flexible 16-bit memory controller supporting asynchronous and synchronous NOR, SRAM, and NAND devices. Hardware timing parameters (`CS`, `OE`, `WE`, and address valid signals) are configured down to individual clock cycles via `GPMC_CONFIG1_i` through `GPMC_CONFIG7_i` per Chip Select line.
* **ELM:** Offloads BCH Error Correction Code (ECC) polynomial root-finding from the ARM core during NAND Flash reads. Supports 4-bit, 8-bit, and 16-bit BCH ECC per 512-byte data block.

**DDR Memory Controller (EMIF4D)**

* **Interface:** Single 16-bit External Memory Interface (EMIF) operating up to 400 MHz (800 MT/s). Supports DDR2, DDR3, DDR3L, and LPDDR.
* **PHY Mechanics:** Handles hardware DQS read/write leveling, temperature-compensated refresh timers, and PHY timing adjustments during board bring-up.

**Programmable Real-time Units (PRU-ICSS) & EDMA3**

| Module | Architectural Highlights |
| --- | --- |
| **PRU-ICSS** | Dual 32-bit 200 MHz RISC cores (PRU0/PRU1) with deterministic 5 ns execution per instruction. Includes direct single-cycle GPIO pin access, 8KB instruction RAM, 8KB data RAM per core, and a local interrupt controller. |
| **EDMA3** | 64-channel DMA engine with 8 independent transfer controllers. Supports 3D memory transfers (A-sync, AB-sync), hardware event triggers (UART, SPI, McASP), and descriptor-link chaining. |

**Power Reset Clock Management (PRCM)**

* **Clock Domains:** Manages digital Phase-Locked Loops (DPLLs) for MPU, Core, DDR, Peripheral, and Display domains.
* **Power Islands:** Controls dynamic clock gating, power domain switching (WKUP, PER, MPU), and SmartIdle handshake protocols with peripherals to minimize active and standby power.

**Interprocessor Communication (IPC): Cortex-A8, PRU, and PRCM**

* **Hardware Mailbox:** 8 hardware queues providing atomic interrupt-driven message passing between Cortex-A8 and PRU cores.
* **Hardware Spinlocks:** 128 hardware locks for thread-safe mutual exclusion over shared system resources.
* **Shared SRAM:** 12KB local PRU shared RAM and 64KB on-chip SRAM accessible directly by Cortex-A8, PRUs, and EDMA3 via the L3 matrix.
* **Control Pipeline:** PRUs raise system events mapped to the AINTC to alert Cortex-A8, while PRCM independently manages clock gating and hardware resets for the PRU-ICSS core domain.


---

# 5 Phased exploration plan
This five-phase execution plan prioritizes register-level mechanics, hardware state transitions, and memory-mapped controls across all nine domains of the TI AM335x SoC.

**Phase 1: Core Architecture & Vector Execution**

* **Cortex-A8 & CBASS Interconnect:** Audit the 13-stage dual-issue superscalar pipeline execution constraints, L1 parity vs. L2 ECC handling, and L3 (200 MHz) vs. L4 (100 MHz/50 MHz) CBASS crossbar routing rules, bus arbitration priorities, and address translation windows.
* **NEON SIMD Engine:** Analyze VFPv3/NEON co-processor pipeline latencies, register bank access patterns (thirty-two 64-bit $D$ vs. sixteen 128-bit $Q$ registers), instruction dual-issue rules, and CPU-to-NEON register transfer penalties to minimize pipeline stalls.

**Phase 2: Deterministic Real-Time & Offload Engines**

* **PRU-ICSS Execution:** Map the dual 200 MHz single-cycle RISC cores, direct bitwise I/O pin mapping via `r30` (outputs) and `r31` (inputs), local 8KB I-RAM / 8KB D-RAM layouts, and local event routing through the PRU interrupt controller.
* **EDMA3 Controller:** Interrogate the 64-channel DMA engine architecture, focusing on Parameter RAM (PaRAM) 32-byte descriptor structure sets, A-sync vs. AB-sync transfer modes, link/chaining mechanisms, and event-trigger mapping from peripherals (UART, McASP, SPI).

**Phase 3: Hardware Interrupts, Power & Clocking**

* **AINTC (ARM Interrupt Controller):** Trace the non-GIC 128-line priority arbitration pipeline, hardware auto-prioritization via `SIR_IRQ` / `SIR_FIQ` registers, nesting controls, and IRQ priority threshold masking.
* **PRCM Engine:** Deconstruct Phase-Locked Loop (DPLL) locked/bypass state transitions across MPU, Core, DDR, and Peripheral domains, power domain logic (WKUP, PER, MPU), dynamic clock-gating registers, and SmartIdle handshake protocols.

**Phase 4: Memory Systems & ECC Acceleration**

* **GPMC & ELM Subsystem:** Configure Chip Select timings using `GPMC_CONFIG1_i` through `7_i` registers for asynchronous/synchronous access, coupled with the Error Locator Module (ELM) hardware polynomial engine for offloading 4-bit, 8-bit, and 16-bit BCH ECC calculations on NAND Flash pages.
* **EMIF4D DDR Controller:** Study the 16-bit PHY interface bringup sequence, DQS read/write leveling hardware loops, refresh control registers, and memory-mapping timing parameter adjustments for DDR2/DDR3/DDR3L.

**Phase 5: Interprocessor Communication (IPC) & Shared Fabric**

* **IPC Infrastructure:** Map the 8-queue Hardware Mailbox interrupt signaling, the 128 Hardware Spinlock mutual-exclusion register modules, and multi-master shared L3 memory access patterns between Cortex-A8, PRUs, and EDMA3.

Which of these five phases would you like to start dissecting down to the specific register bits and assembly sequences first?


# Low level exploration
- Watchdog timer clock of AM335x is configured through PRCM

Sequence of steps followed in bootloader initialization code:
0. Board initialization
1. Enable WDT module clock in PRCM
2. Set PLL0 to generate 300MHz for the ARM Core 
3. Initialize DDR 
4. Enable PRU_ICSS module through PRCM
5. Enable timer4 : Used for sys/bios applications