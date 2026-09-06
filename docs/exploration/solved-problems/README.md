# Master Architecture Hub: Solved Engineering Problems & Silicon Bringup

Welcome to the **Solved Engineering Problems & Silicon Bringup Reference Hub**. This repository domain archives practical hands-on engineering challenges, bringup diagnostic workflows, firmware validation tools, and recovery procedures.

It bridges theoretical SoC boot and low-level C programming concepts with real-world silicon hardware troubleshooting.

---

## 1. Problem Index & Domain Topology

```
 +---------------------------------------------------------------------------------------------------+
 | PROBLEM 1: ARMv8 SoC BOOT STAGE & KERNEL HANDOFF VALIDATOR                                       |
 |   • Target: Low-level C/C++ firmware validation engine for ARMv8 boot stages                      |
 |   • Key Concepts: Invariant state validation (x0 DTB pointer, MMU/Cache state, PSTATE masks)    |
 |     ├─► Problem Statement: [Kernel Handoff Validator Spec](1-kernel-handoff-validator/problem-statement.md)
 |     ├─► Proposed Solution & Walkthrough: [Validator Implementation Guide](1-kernel-handoff-validator/proposed-solution.md)
 |     ├─► C Source: [kernel_handoff_validator.c](1-kernel-handoff-validator/solution/kernel_handoff_validator.c)
 |     └─► C++ Source: [kernel_handoff_validator.cpp](1-kernel-handoff-validator/solution/kernel_handoff_validator.cpp)
 +---------------------------------------------------------------------------------------------------+
                                                   │
                                                   ▼
 +---------------------------------------------------------------------------------------------------+
 | PROBLEM 2: RASPBERRY PI 4B SILICON BRINGUP & EEPROM RECOVERY                                      |
 |   • Target: Hardware bringup, boot sequence diagnosis, and SPI EEPROM recovery over USB-C         |
 |   • Key Concepts: BCM2711 Mask ROM (Stage 0), SPI EEPROM (Stage 1), BPB FAT32 alignment, rpiboot   |
 |     ├─► Bringup Diagnosis Log: [RPi 4 Bringup Notes](2-raspberry-pi/bringup.md)                   |
 |     ├─► Phase-by-Phase Procedure: [RPi 4 Bringup Procedure Guide](2-raspberry-pi/bringup_procedure.md)|
 |     ├─► Recovery Skill & Operator Manual: [RPi4 EEPROM Recovery Skill](2-raspberry-pi/rpi4-eeprom-recovery/SKILL.md)
 |     ├─► BPB Alignment Reference: [FAT32 Hidden Sectors & BPB Specs](2-raspberry-pi/rpi4-eeprom-recovery/references/bpb-alignment-and-fat32.md)
 |     ├─► Diagnostics Reference: [BCM2711 Boot Flow & Hardware Diagnostics](2-raspberry-pi/rpi4-eeprom-recovery/references/diagnostics-and-troubleshooting.md)
 |     ├─► Recovery Payloads Spec: [EEPROM Payload ZIP Variations](2-raspberry-pi/rpi4-eeprom-recovery/references/recovery-payloads.md)
 |     └─► EEPROM Release Notes: [Official Firmware Release Notes](2-raspberry-pi/EEPROM_recovery_payload/release-notes.md)
 +---------------------------------------------------------------------------------------------------+
```

---

## 2. Summary of Engineering Exercises

### Problem 1: ARMv8 SoC Boot Stage & Kernel Handoff Validator
* **Context:** In ARMv8 boot sequences (BL1 $\rightarrow$ BL2 $\rightarrow$ BL31 $\rightarrow$ BL33 $\rightarrow$ Kernel), handed-off CPU hardware states must satisfy exact architectural invariants before control enters `head.S`.
* **Verification:** The C/C++ validator checks register states (`x0` pointing to FDT magic `0xd00dfeed`), MMU/Cache state, and PSTATE interrupt masks (`.D`, `.A`, `.I`, `.F`).

### Problem 2: Raspberry Pi 4B Silicon Bringup & EEPROM Recovery
* **Context:** Diagnosing silent boot failures (single LED flash, uninitialized SPI EEPROM bootloader) on Broadcom BCM2711 SoCs.
* **Resolution:** Utilizing `rpiboot` recovery mode over USB-C to reflash the onboard SPI EEPROM chip with FAT32 BPB-aligned payloads (`BOOT_ORDER=0xf41`).

---

## 3. Cross-Domain Master Links

* [SoC Boot Sequence & Hardware Bringup Pipeline Hub](../concepts/HUB_ARM-Cortex-A-Boot-Process.md)
* [Core Computing, Memory Hierarchy & Accelerators Hub](../concepts/HUB_Core_Computing.md)
* [Linux Kernel Architecture & Memory Management Hub](../linux_kernel/hub_kernel.md)
* [Systems C/C++, Pointers & Concurrency Hub](../c_c++/hub_concepts.md)
* [Barebox Bootloader Architecture](../tools/Barebox.md)
* [Boot Troubleshooting FAQ](../questions/boot.md)
