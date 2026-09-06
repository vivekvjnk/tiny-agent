# Diagnostics & Troubleshooting Reference

## Hardware Boot Flow of BCM2711 (Raspberry Pi 4B)

The Broadcom BCM2711 SoC follows a two-stage hardware boot process:

1. **Stage 0 (Mask ROM):** Hardcoded in silicon at `0x00000000`. At power-on, Mask ROM initializes minimal CPU state and attempts to load the bootloader payload from the onboard SPI EEPROM chip.
2. **Stage 1 (SPI EEPROM):** Firmware stored in the 512KB SPI EEPROM chip. Contains logic to initialize RAM, PCIe (for USB host), and find `start4.elf` on SD/USB storage.

---

## Diagnostic Decision Tree via Green ACT LED

Remove all SD cards and USB drives from the Pi 4B, connect power, and observe the green ACT LED:

| Green ACT LED Pattern | Hardware State | Primary Diagnosis | Corrective Action |
|---|---|---|---|
| **4 Blinks (Repeated with delay)** | **EEPROM Healthy** | Bootloader is functioning properly and searching for `start4.elf`. | Issue is strictly on the OS SD card (partitioning, `start4.elf`, or card contact). |
| **Solid Green LED (No blinks)** | **Mask ROM Lockup / SD Fail** | Mask ROM cannot parse SD card filesystem OR EEPROM bootloader is corrupt/missing. | Perform Direct USB-C Recovery via `rpiboot`. |
| **Single Blink / Solid OFF** | **EEPROM Corrupted** | Mask ROM failed SPI EEPROM validation check. | Reflash EEPROM bootloader. |

---

## Why USB Drive Recovery Fails When EEPROM is Corrupted

- **Hardware Constraint:** The BCM2711 Mask ROM (Stage 0) **does NOT support USB Host mode**.
- USB host support (USB-A 2.0/3.0 ports) is implemented entirely in software inside the **SPI EEPROM firmware (Stage 1)**.
- If the SPI EEPROM is corrupted or uninitialized, inserting a USB flash drive into the USB-A ports will **never** trigger recovery.

---

## Why 64GB+ SDXC Cards Cause Solid Green LED

- High-capacity SDXC cards (64GB, 128GB+) require high-speed UHS-I voltage/clock handshakes (3.3V to 1.8V transition).
- Before EEPROM firmware is loaded, Stage 0 Mask ROM operates in legacy SD 1-bit mode at 3.3V and frequently fails clock negotiation on SDXC cards, hanging the Mask ROM with a solid green LED.

---

## Comparison: SD Recovery vs USB-C Direct Recovery (`rpiboot`)

| Feature | SD Card Recovery | Direct USB-C Recovery (`rpiboot`) |
|---|---|---|
| **Interface Used** | MicroSD Card Slot | USB-C Power Port (USB Device Mode) |
| **Stage 0 Dependency** | Depends on SD slot pin contact & SDXC negotiation | Uses built-in BCM2711 USB Device gadget mode |
| **Bypasses SD Slot?** | No | **Yes** |
| **Reliability** | Medium (fails if BPB or SDXC card issue exists) | **100% Guaranteed** |
