# Barebox Bootloader Architecture & Reference

Barebox (formerly `u-boot-v2`) is an open-source, multi-architecture bootloader designed specifically for embedded Linux systems. Unlike traditional bootloaders, Barebox adopts the Linux kernel's driver model (`struct driver_d`, `struct device_d`), Virtual File System (VFS), Device Tree handling, and shell environment.

---

## 1. Architectural Comparison: Barebox vs U-Boot

| Architectural Metric | Barebox | U-Boot |
| --- | --- | --- |
| **Driver Framework** | Modeled directly on Linux Kernel driver model (`probe()`, `remove()`, `bus_type`). | Custom `driver model` (DM) API abstraction. |
| **Virtual File System** | Complete Linux-like VFS (`/dev/mem`, `/dev/disk0`, `/env/`, `/mnt/`). | Command-based raw partition/fs access (`fatload`, `ext4load`). |
| **Device Tree Integration** | Native internal DTS usage for both bootloader hardware and kernel handoff. | Hybrid model (DT used for DM drivers, legacy headers for board config). |
| **Multi-Image Binaries** | Generates a single binary supporting multiple board targets via Pre-Bootloader (PBL). | Requires separate compiled binary images per board variant. |
| **Shell & Scripting** | Advanced POSIX-like shell (`hush`) with environment variables under `/env/`. | Custom U-Boot command parser / basic hush shell. |

---

## 2. Barebox Binary Lifecycle & PBL Architecture

```
 +---------------------------------------------------------------------------------+
 | PRE-BOOTLOADER (PBL) - Runs in Internal SRAM                                    |
 |                                                                                 |
 |  • Board-specific early init (UART console, low-level clocks)                   |
 |  • DDR PHY Initialization & Training                                           |
 |  • Decompresses Barebox Proper binary into main DRAM                            |
 +---------------------------------------------------------------------------------+
                                         │
                                         ▼
 +---------------------------------------------------------------------------------+
 | BAREBOX PROPER - Runs in Main DRAM                                              |
 |                                                                                 |
 |  • Initializes VFS (`/dev/`, `/env/`, `/mnt/`)                                  |
 |  • Probes DTS driver hierarchy                                                  |
 |  • Parses Bootspec configurations (`/loader/entries/`)                          |
 |  • Loads and branches to Linux Kernel                                           |
 +---------------------------------------------------------------------------------+
```

---

## 3. Bootspec Standard Specification

Barebox natively implements the **Freedesktop Boot Loader Specification (Bootspec)**. Boot entries are plain text files stored under `/loader/entries/*.conf` in the boot partition:

```ini
title        Linux Kernel Enterprise
version      6.6.21-rt
options      console=ttyS0,115200 earlycon=uart8250,mmio32,0xFE215000 root=/dev/mmcblk0p2 rw
linux        /boot/Image
devicetree   /boot/bcm2711-rpi-4-b.dtb
```

### Bootspec Key Parameters

| Key | Mandatory / Optional | Description |
| --- | --- | --- |
| `title` | **Mandatory** | Human-readable OS menu description. |
| `version` | Optional | Kernel / release version string. |
| `options` | Optional | Kernel command line arguments (`bootargs`). |
| `linux` | **Mandatory** | Path to the kernel binary relative to boot filesystem root. |
| `devicetree` | Optional | Path to the DTB file relative to boot filesystem root. |
| `initrd` | Optional | Path to initial RAM disk / initramfs image. |

---

## 4. High-Entropy Command Reference Table

| Command | Usage Example | Technical Action |
| --- | --- | --- |
| `devinfo` | `devinfo uart0` | Dumps hardware resource allocations (MMIO range, IRQ lines, clock rates) for a device node. |
| `i2c_probe` | `i2c_probe -b 0` | Scans designated I2C bus `0` and prints active client peripheral addresses. |
| `of_dump` | `of_dump /soc/uart@fe215000` | Displays runtime Device Tree node properties and values currently parsed in memory. |
| `global` | `global.boot.default=mmc` | Sets or displays global system environment variables. |
| `boot` | `boot net` or `boot /mnt/sd` | Triggers Bootspec auto-discovery and executes default boot sequence. |
| `memcpy` | `memcpy -s /dev/nor0 -d /dev/ram0 0 0 0x100000` | Directly streams bytes between device files in VFS space. |
