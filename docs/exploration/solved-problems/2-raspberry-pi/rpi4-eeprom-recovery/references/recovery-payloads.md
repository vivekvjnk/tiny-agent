# EEPROM Recovery Payloads Reference

## Package Variations: `-default.zip` / `-sd.zip` vs `-network.zip`

Official Raspberry Pi firmware releases contain different EEPROM recovery zip variations:

| File Pattern | Primary Purpose | Default `BOOT_ORDER` | Result on Power-On |
|---|---|---|---|
| `rpi-boot-eeprom-recovery-*-default.zip` / `-sd.zip` | **Standard SD Card Boot Recovery** | `0xf41` (SD Card primary, USB secondary, restart loop) | Restores standard boot. Boots SD card automatically. |
| `rpi-boot-eeprom-recovery-*-network.zip` | **Network Install Mode Recovery** | `0xf21` or HTTP NetInstall mode | Forces Network/HTTP Boot screen over Ethernet. Fails without network server. |

> **CRITICAL:** Do NOT flash `-network.zip` if you intend to boot Raspberry Pi OS locally from MicroSD card. Always use `-default.zip` or `-sd.zip`.

---

## Required Files in Recovery Directory

For `rpiboot` or SD card recovery to execute successfully, the root directory must contain:

```text
recovery_dir/
├── bootcode4.bin -> recovery.bin   # Symlink required by rpiboot for Pi 4 (2711)
├── recovery.bin                   # Primary recovery binary executed in SRAM
├── pieeprom.bin                   # SPI EEPROM binary image
├── pieeprom.upd                   # Copy of pieeprom.bin (checked by recovery.bin)
├── pieeprom.sig                   # SHA-256 signature of pieeprom.bin
├── pieeprom.sig.upd               # Copy of pieeprom.sig
├── vl805.bin                      # USB 3.0 VL805 PCIe controller firmware
└── vl805.sig                      # Signature for VL805 firmware
```

---

## Default EEPROM Configuration Inspection (`bootconf.txt`)

Inside `pieeprom.bin`, the embedded configuration (`[all]` section) for standard SD boot should match:

```ini
[all]
BOOT_UART=0
WAKE_ON_GPIO=1
POWER_OFF_ON_HALT=0
ENABLE_SELF_UPDATE=1
BOOT_ORDER=0xf41
NET_INSTALL_AT_POWER_ON=1
```

Where:
- `BOOT_ORDER=0xf41`: Try SD card (`0x1`), try USB mass storage (`0x4`), restart loop (`0xf`).
