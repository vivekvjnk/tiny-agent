# ARM Cortex-A Cold Boot Sequence & Silicon Bringup Architecture

This document provides a hardware and firmware reference for the end-to-end boot sequence on Armv8/Armv9 Cortex-A Application Processors, from hardware reset and PMIC rail stabilization up to Linux kernel entry.

---

## 1. Complete System Boot Pipeline

```
 +-----------------------------------------------------------------------------------+
 | PHASE 1: HARDWARE INITIALIZATION                                                  |
 |                                                                                   |
 |  [Power Rail Ramp] ---> [Crystal Osc Lock] ---> [PLL Lock] ---> [Release POR_N]   |
 +-----------------------------------------------------------------------------------+
                                           │
                                           ▼
 +-----------------------------------------------------------------------------------+
 | PHASE 2: TRUSTED FIRMWARE-A EXECUTION (TF-A)                                      |
 |                                                                                   |
 |  +---------------------+      Loads      +---------------------+                  |
 |  |  BootROM / BL1      | --------------> | BL2 (Secure EL1)    |                  |
 |  |  (Secure EL3 SRAM)  |                 | (Internal SRAM)     |                  |
 |  +---------------------+                 +---------------------+                  |
 |                                                     │                             |
 |                                                     │ Initializes DDR PHY         |
 |                                                     │ Authenticates Downstream    |
 |                                                     v                             |
 |  +---------------------+                 +---------------------+                  |
 |  | BL32 (OP-TEE SEL1)  | <-------------- | BL31 (Secure EL3)   |                  |
 |  | (Secure DRAM)       |      Loads      | (Runtime Monitor)   |                  |
 |  +---------------------+                 +---------------------+                  |
 +-----------------------------------------------------│-----------------------------+
                                                       │ Passes Execution
                                                       v
 +-----------------------------------------------------------------------------------+
 | PHASE 3: NON-SECURE OS BOOT                                                       |
 |                                                                                   |
 |  +---------------------+                 +---------------------+                  |
 |  | BL33 (U-Boot/Bare)  | --------------> | Linux Kernel Image  |                  |
 |  | (Non-Secure EL2)    |   Loads & Jumps | (Non-Secure EL1)    |                  |
 |  +---------------------+                 +---------------------+                  |
 +-----------------------------------------------------------------------------------+
```

---

## 2. Hardware Power-On Reset (POR) & PMIC Ramp Sequence

Before the CPU executes its first instruction from BootROM, hardware rail sequencing must complete cleanly to avoid CMOS latch-up or flash corruption:

```
Voltage / Signals
  ▲
  │
  ├─ VDD_CORE  ─────────────┌─────────────────────────────────────────────── (Core Logic Supply)
  │                         │
  ├─ VDD_MEM   ─────────┌───┘                                                 (DRAM/SRAM Supply)
  │                     │
  ├─ VDD_IO    ─────┌───┘                                                     (Peripherals/I-O)
  │                 │
  ├─ SYS_CLK   ─────┼───────┌─┐─┌─┐─┌─┐─┌─┐─┌─┐─┌─┐─┌─┐─┌─┐─────────────────── (Crystal/PLL Stable)
  │                 │       │ │ │ │ │ │ │ │ │ │ │ │ │ │ │
  ├─ POR_N     ─────┴───────┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─└───┐───────────────── (Power-On Reset Released)
  │                                                         │
  └─────────────────────────────────────────────────────────┴─────────────────► Time
                                                            CPU Fetch Starts
```

### PMIC State Machine Steps

1. **Power Rail Ramp:** PMIC regulators ramp in hardwired sequence: $V_{\text{DD\_CORE}} \rightarrow V_{\text{DD\_MEM}} \rightarrow V_{\text{DD\_IO}}$ to prevent parasitic di/dt latch-up.
2. **Clock Stabilization:** Crystal oscillator turns on $\rightarrow$ Main System PLL locks to target frequency $\rightarrow$ Power-Good (`PG`) output signal asserted by PMIC.
3. **Reset De-assertion:** System controller releases `POR_N` (Power-On Reset, active low) signal.
4. **Boot Pin Sampling:** Core 0 samples hardware bootstrap pins (latch configuration for boot device: eMMC, SPI NOR, USB, or PCIe Endpoint mode).
5. **Instruction Fetch:** Core 0 un-clears reset vector and fetches first arm64 instruction from hardcoded On-Chip BootROM address (`0x0000_0000` or vendor vector).

---

## 3. Comprehensive Stage Execution Matrix

| Stage | Software Component | Execution Level | Memory Context | Key Responsibility | Handoff Trigger |
| --- | --- | --- | --- | --- | --- |
| **BL1** | Primary BootROM | Secure EL3 | Internal ROM / SRAM | Validates boot pin straps, initializes SPI/eMMC drivers, loads & authenticates BL2 image into SRAM. | Direct Branch |
| **BL2** | Trusted Boot Firmware | Secure EL1 / EL3 | Internal SRAM (128KB-512KB) | Configures PLL clock trees, PMIC voltage registers, executes **DDR PHY Calibration**, authenticates BL31/BL32/BL33. | Return to BL1 / SMC |
| **BL31** | TF-A Runtime Monitor | Secure EL3 | Secure SRAM / DRAM | Remains resident in memory. Serves PSCI calls, handles SMC exceptions, switches execution world. | `ERET` to EL2 |
| **BL32** | Secure Payload | Secure EL1 | Secure DRAM (TZASC) | Optional OP-TEE / Trusted OS for crypto key management and hardware isolation. | SMC Return |
| **BL33** | Non-Secure Bootloader | Non-Secure EL2 | Main DRAM | U-Boot / Barebox. Parses DTB, loads kernel image into DRAM, sets CPU registers for OS boot. | Branch (`br x4`) |
| **Kernel** | Linux Kernel (`head.S`) | Non-Secure EL1 | Main DRAM | Sets up page tables, enables MMU, probes DTB devices, initializes user space (`init`). | N/A |

---

## 4. Kernel Entry Hardware State Contract (`head.S`)

When BL33 branches to the Linux kernel entry point, the CPU hardware state must strictly satisfy this architectural contract:

| Hardware Constraint | Architectural Requirement | Reason / Failure Mode |
| --- | --- | --- |
| **`x0` Register** | **Physical address of `.dtb`** | Points to Device Tree Blob in DRAM. Invalid address causes kernel panic during early boot. |
| **`x1`, `x2`, `x3` Regs** | **Must be `0`** | Reserved for future architecture specification extensions. |
| **MMU Status** | **Disabled** | Page tables are not yet configured; enabled MMU causes immediate MMU Translation Fault. |
| **Data Cache (D-Cache)** | **Disabled** | Prevents stale memory hits before virtual memory paging and cache maintenance are initialized. |
| **Instruction Cache** | **Enabled or Disabled** | I-cache may be enabled for boot speedup; coherent with physical DRAM. |
| **PSTATE Interrupt Flags** | **Masked (`.D`, `.A`, `.I`, `.F` = 1)** | All IRQs, FIQs, SError, and Debug exceptions masked. Handler vector table `VBAR_EL1` is uninitialized. |
| **Execution Level** | **EL2 or EL1** | Booting at EL2 allows KVM hypervisor initialization; EL1 is standard non-secure boot. |
| **Endianness** | **Little Endian (EE bit = 0)** | Standard Linux AArch64 kernel expects Little Endian execution context. |
