# Boot Sequence & Silicon Bringup Architecture: FAQ & Troubleshooting

This document provides concise technical explanations for critical boot sequence, firmware handoff, and hardware bringup scenarios.

---

## Technical Questions & Architectural Answers

### Q1: Why must MMU and Data Caches be disabled at Linux Kernel entry (`head.S`)?

| Reason / Risk | Architectural Explanation |
| --- | --- |
| **Page Tables Uninitialized** | Linux kernel virtual-to-physical mapping tables are built dynamically during `setup_arch()`. Enabling MMU prior to this causes immediate Translation Faults. |
| **Memory Coherency Hazard** | Data Caches enabled without active virtual memory management cause incoherent cacheline hits between physical DRAM and CPU cache controllers, risking silent memory corruption. |

---

### Q2: How do PSCI and SCMI differ in Arm Systems Architecture?

```
 +---------------------------------------------------------------------------------+
 | LINUX KERNEL                                                                    |
 |   • CPU Power Management Drivers -------------> Calls PSCI (via smc #0)          |
 |   • Clock / Thermal / Voltage Drivers --------> Calls SCMI (via Mailbox/SMC)     |
 +---------------------------------------------------------------------------------+
                               │                                │
                               ▼                                ▼
 +---------------------------------------------------------------------------------+
 | FIRMWARE SUBSYSTEM                                                              |
 |   • TF-A (Secure EL3) ------------------------> Executes PSCI (CPU_ON, SUSPEND)   |
 |   • System Control Processor (SCP Firmware) --> Executes SCMI (Clocks/Regulators)|
 +---------------------------------------------------------------------------------+
```

| Aspect | Power State Coordination Interface (PSCI) | System Control & Management Interface (SCMI) |
| --- | --- | --- |
| **Primary Scope** | CPU core lifecycle management (`CPU_ON`, `CPU_OFF`, `CPU_SUSPEND`, `SYSTEM_RESET`). | System-wide peripheral management (Clocks, Power domains, Sensors, Reset lines, DVFS). |
| **Transport Mechanism** | Direct `SMC` / `HVC` instruction calls into TF-A (EL3). | Shared SRAM memory buffers + Mailbox doorbells / SMC triggers to System Control Processor (SCP). |
| **Specification Owner** | Arm Infrastructure Specifications | Arm System Control & Management Interface |

---

### Q3: Why can DDR PHY Training NOT occur in U-Boot or Linux?

* **DRAM Dependency Catch-22:** Multi-megabyte binaries like U-Boot and Linux require functional physical DRAM to execute.
* **SRAM Boot Solution:** BL2 executes strictly inside isolated, deterministic On-Chip SRAM (typically 128KB-512KB). It initializes the DDR PHY controller, running DQS read/write leveling, eye-diagram training, and $V_{\text{REF}}$ calibration. Once training completes, main DRAM becomes operational and downstream binaries (BL31, BL33) are fetched into DRAM.

---

### Q4: Root Cause Analysis Matrix: Silent Hang during BL2 Boot

| Hardware / Firmware Failure Mode | Diagnostic Procedure & Root Cause |
| --- | --- |
| **DDR PHY Calibration Lockup** | **Symptom:** Board hangs silently before serial log initialization.<br>**Root Cause:** Impedance mismatch, poor signal integrity, or bad $V_{\text{REF}}$ calibration causing endless hardware leveling loops. |
| **ROTPK / Cryptographic Auth Failure** | **Symptom:** BL1 halts execution without loading BL2.<br>**Root Cause:** Public key hash mismatch between OTP fuse bank and BL2 image signature. |
| **PMIC Rail Collapse** | **Symptom:** System powers down abruptly when DDR PHY turns on.<br>**Root Cause:** Excessive inrush current on $V_{\text{DD\_MEM}}$ rail exceeding PMIC current limit thresholds. |

---

### Q5: What is the exact Register Handoff State when BL33 calls Linux Kernel?

| Register | Contract Value | Meaning / Purpose |
| --- | --- | --- |
| **`x0`** | `Physical Memory Address` | Points to Flattened Device Tree Blob (`.dtb`) in DRAM. |
| **`x1`, `x2`, `x3`** | `0x00000000` | Reserved for future architecture specification extensions. |
| **`PSTATE`** | `0x3C5` (`.D=.A=.I=.F=1`) | All asynchronous interrupts (Debug, SError, IRQ, FIQ) masked. |
| **`SCTLR_EL1`** | `Bit 0 (M) = 0, Bit 2 (C) = 0` | MMU Disabled, Data Cache Disabled. |
