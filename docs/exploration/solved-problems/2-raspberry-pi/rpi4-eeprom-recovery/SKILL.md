---
name: rpi4-eeprom-recovery
description: >
  Reflashes or recovers the Raspberry Pi 4B SPI EEPROM bootloader directly over USB-C using rpiboot.
  Use when a Pi 4 fails to boot, shows a solid green ACT LED, has a corrupted bootloader, or was accidentally
  flashed with network recovery bootloader (-network.zip). Triggers include "Raspberry Pi 4 failed to boot",
  "Pi 4 solid green LED", "reflash Pi 4 EEPROM", or "corrupted RPi 4 bootloader".
---

# Raspberry Pi 4B EEPROM Recovery

## Overview
This capability provides the **guaranteed happy path** for reflashing and restoring the Raspberry Pi 4B onboard SPI EEPROM bootloader using direct USB-C connection (`rpiboot`). Bypassing SD card slot issues and SDXC voltage negotiation failures, it pushes `recovery.bin` directly into BCM2711 SRAM to reflash the EEPROM.

---

## When to Use This Skill
- Raspberry Pi 4B shows a **solid green ACT LED** when powered on with an OS SD card or recovery card.
- Pi 4B shows a **single blink or solid OFF** green LED when powered on with NO SD card inserted.
- The EEPROM was accidentally flashed with a network recovery payload (`rpi-boot-eeprom-recovery-*-network.zip`), causing the board to hang waiting for Ethernet/HTTP install.
- The board fails to read MicroSD cards due to corrupted SPI EEPROM bootloader state.

> **DO NOT USE FOR:** Raspberry Pi 1, 2, 3, Zero, or Pi 5 (Pi 4B uses BCM2711 architecture).

---

## Inputs & Prerequisites
1. **Target Board:** Raspberry Pi 4B model B.
2. **Host Workstation:** Linux PC with USB port and `sudo` access.
3. **Hardware Cable:** USB cable (USB-A to USB-C or USB-C to USB-C) connected directly to the Pi 4B **USB-C power port**.
4. **Tools Needed:** `rpiboot` (built from official `raspberrypi/usbboot` repository).
5. **No SD Card Attached:** Ensure the MicroSD card slot on the Pi 4B is completely empty during flashing.

---

## Happy Path Workflow (Direct USB-C Recovery)

### Step 1: Prepare the Target Pi 4B Board
1. **Unplug power** from the Raspberry Pi 4B.
2. **Remove the MicroSD card** completely from the SD slot.
3. **Disconnect all peripherals** (USB flash drives, HDMI, Ethernet).

### Step 2: Prepare Recovery Payload
Ensure the recovery payload directory (e.g. `/tmp/rpiboot_recovery`) contains standard SD boot recovery files (`-sd.zip` or `-default.zip`):
- `recovery.bin`
- `bootcode4.bin` (symlinked to `recovery.bin`)
- `pieeprom.bin` & `pieeprom.upd`
- `pieeprom.sig` & `pieeprom.sig.upd`
- `vl805.bin` & `vl805.sig`

*See [references/recovery-payloads.md](references/recovery-payloads.md) for details on downloading and verifying payloads.*

### Step 3: Connect Pi 4B to Host PC via USB-C
1. Connect a USB cable from a USB port on your **Linux PC** directly into the **USB-C power port** of the Raspberry Pi 4B.
2. *(The host PC powers the Pi 4B and BCM2711 Mask ROM enters USB Device mode `0a5c:2711`).*

### Step 4: Execute `rpiboot` Flashing Script
Run the automated flash helper tool:

```bash
sudo ./tools/rpiboot_flash.sh
```

Or run `rpiboot` directly:

```bash
sudo rpiboot -v -d /tmp/rpiboot_recovery
```

**Expected Output:**
```text
Device located successfully
Loading: /tmp/rpiboot_recovery/bootcode4.bin
Initialised device correctly
Writing 100140 bytes...
Successful read 4 bytes
```

### Step 5: Observe Hardware LED Confirmation
1. Watch the Green ACT LED on the Pi 4B board.
2. `recovery.bin` will execute in SRAM, reflash the SPI EEPROM, and transition the ACT LED into a **rapid, steady blinking pattern**.
3. **Unplug the USB-C cable** once continuous rapid blinking is established.

---

## Verification & Success Criteria

1. **Power-On Self Test (NO SD Card):**
   - Connect power supply to Pi 4B with NO SD card inserted.
   - **SUCCESS CRITERIA:** Green ACT LED blinks **4 times repeatedly with a short pause**.
   - *(4 blinks confirms the SPI EEPROM bootloader is healthy and actively searching for `start4.elf`).*

2. **OS Boot Test:**
   - Insert a valid Raspberry Pi OS MicroSD card into the Pi 4B and power on.
   - The Pi 4B loads `start4.elf` and boots into Raspberry Pi OS cleanly.

---

## Deep Technical References

- **Diagnostics & Decision Trees:** [references/diagnostics-and-troubleshooting.md](references/diagnostics-and-troubleshooting.md)
- **FAT32 BPB Alignment & Geometry:** [references/bpb-alignment-and-fat32.md](references/bpb-alignment-and-fat32.md)
- **Payload Types & Differences:** [references/recovery-payloads.md](references/recovery-payloads.md)
