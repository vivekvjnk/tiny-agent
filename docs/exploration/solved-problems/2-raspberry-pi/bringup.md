# Bug 1
- Created bootable SD-Card using "Raspbery Pi Imager" tool.
- Mount SD-Card in Raspberry Pi 4 Model B and started the device
- Device is not booting from SD-Card
    - Green LED is blinking only once
    - Red LED is stable 
    
# Steps to debug
- Pi is in headless mode
- First we analyze the SD Card
## SD Card integrity check
1. Verify partition table & file system
```bash
$ sudo fsblk -l /dev/mmcblk0
Disk /dev/mmcblk0: 57.95 GiB, 62226694144 bytes, 121536512 sectors
Units: sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 512 bytes
Disklabel type: dos
Disk identifier: 0x36f6e378

Device         Boot   Start      End  Sectors  Size Id Type
/dev/mmcblk0p1        16384  1064959  1048576  512M  c W95 FAT32 (LBA)
/dev/mmcblk0p2      1064960 12681215 11616256  5.5G 83 Linux
```
Required conditions:
- Disk label type must be dos (MBR), not gpt.
- Partition 1 must be formatted as FAT32 (vfat) with partition type 0x0c (W95 FAT32 LBA) or 0x0b.

Both are satisfied 

2. Check filesystem corruption
- Unmount all partitions
- Check file system corruption
```bash
$ sudo fsck.vfat -v -a /dev/mmcblk0p1
fsck.fat 4.2 (2021-01-31)
Checking we can access the last sector of the filesystem
Boot sector contents:
System ID "mkfs.fat"
Media byte 0xf8 (hard disk)
       512 bytes per logical sector
       512 bytes per cluster
        32 reserved sectors
First FAT starts at byte 16384 (sector 32)
         2 FATs, 32 bit entries
   4129792 bytes per FAT (= 8066 sectors)
Root directory start at cluster 2 (arbitrary size)
Data area starts at byte 8275968 (sector 16164)
   1032408 data clusters (528592896 bytes)
63 sectors/track, 255 heads
     16384 hidden sectors
   1048572 sectors total
Reclaiming unconnected clusters.
Checking free cluster summary.
/dev/mmcblk0p1: 424 files, 177810/1032408 clusters

$ sudo fsck.vfat -v -a /dev/mmcblk0p2
fsck.fat 4.2 (2021-01-31)
Logical sector size is zero.
```

3. Check boot partition files 
- Mount boot partition

```bash
± |vivek/low_level_exploration U:1 ?:3 ✗| → sudo mkdir /mnt/sdcard_boot
± |vivek/low_level_exploration U:1 ?:3 ✗| → sudo mount /dev/mmcblk0p1 /mnt/sdcard_boot/
± |vivek/low_level_exploration U:1 ?:3 ✗| → sudo mount /dev/mmcblk0p2 /mnt/sdcard
± |vivek/low_level_exploration U:1 ?:3 ✗| → lsblk
NAME        MAJ:MIN RM   SIZE RO TYPE MOUNTPOINTS
mmcblk0     179:0    0    58G  0 disk 
├─mmcblk0p1 179:1    0   512M  0 part /mnt/sdcard_boot
└─mmcblk0p2 179:2    0   5.5G  0 part /mnt/sdcard
zram0       251:0    0     8G  0 disk [SWAP]
nvme0n1     259:0    0 465.8G  0 disk 
├─nvme0n1p1 259:2    0   600M  0 part /boot/efi
├─nvme0n1p2 259:3    0     2G  0 part /boot
└─nvme0n1p3 259:4    0 463.2G  0 part /home
                                      /
nvme1n1     259:1    0 476.9G  0 disk 
├─nvme1n1p1 259:5    0   260M  0 part 
├─nvme1n1p2 259:6    0    16M  0 part 
├─nvme1n1p3 259:7    0 474.7G  0 part 
└─nvme1n1p4 259:8    0     2G  0 part 
± |vivek/low_level_exploration U:1 ?:3 ✗| → ls -la /mnt/sdcard_boot/
total 87654
drwxr-xr-x. 4 root root     4096 Jan  1  1970  .
drwxr-xr-x. 1 root root       34 Aug 20 07:28  ..
-rwxr-xr-x. 1 root root    32539 Jun  9 19:25  bcm2710-rpi-2-b.dtb
-rwxr-xr-x. 1 root root    34731 Jun  9 19:25  bcm2710-rpi-3-b.dtb
-rwxr-xr-x. 1 root root    35252 Jun  9 19:25  bcm2710-rpi-3-b-plus.dtb
-rwxr-xr-x. 1 root root    33720 Jun  9 19:25  bcm2710-rpi-cm0.dtb
-rwxr-xr-x. 1 root root    32302 Jun  9 19:25  bcm2710-rpi-cm3.dtb
-rwxr-xr-x. 1 root root    33708 Jun  9 19:25  bcm2710-rpi-zero-2.dtb
-rwxr-xr-x. 1 root root    33708 Jun  9 19:25  bcm2710-rpi-zero-2-w.dtb
-rwxr-xr-x. 1 root root    56411 Jun  9 19:25  bcm2711-rpi-400.dtb
-rwxr-xr-x. 1 root root    56407 Jun  9 19:25  bcm2711-rpi-4-b.dtb
-rwxr-xr-x. 1 root root    56928 Jun  9 19:25  bcm2711-rpi-cm4.dtb
-rwxr-xr-x. 1 root root    39913 Jun  9 19:25  bcm2711-rpi-cm4-io.dtb
-rwxr-xr-x. 1 root root    53824 Jun  9 19:25  bcm2711-rpi-cm4s.dtb
-rwxr-xr-x. 1 root root    78679 Jun  9 19:25  bcm2712d0-rpi-5-b.dtb
-rwxr-xr-x. 1 root root    78675 Jun  9 19:25  bcm2712-d-rpi-5-b.dtb
-rwxr-xr-x. 1 root root    78631 Jun  9 19:25  bcm2712-rpi-500.dtb
-rwxr-xr-x. 1 root root    78675 Jun  9 19:25  bcm2712-rpi-5-b.dtb
-rwxr-xr-x. 1 root root    79620 Jun  9 19:25  bcm2712-rpi-cm5-cm4io.dtb
-rwxr-xr-x. 1 root root    79686 Jun  9 19:25  bcm2712-rpi-cm5-cm5io.dtb
-rwxr-xr-x. 1 root root    79661 Jun  9 19:25  bcm2712-rpi-cm5l-cm4io.dtb
-rwxr-xr-x. 1 root root    79727 Jun  9 19:25  bcm2712-rpi-cm5l-cm5io.dtb
-rwxr-xr-x. 1 root root    52624 Jun 18 05:50  bootcode.bin
-rwxr-xr-x. 1 root root      221 Aug 18 19:21  cmdline.txt
-rwxr-xr-x. 1 root root     1272 Jun 18 05:50  config.txt
-rwxr-xr-x. 1 root root     3273 Jun 18 05:50  fixup4cd.dat
-rwxr-xr-x. 1 root root     5499 Jun 18 05:50  fixup4.dat
-rwxr-xr-x. 1 root root     8494 Jun 18 05:50  fixup4db.dat
-rwxr-xr-x. 1 root root     8494 Jun 18 05:50  fixup4x.dat
-rwxr-xr-x. 1 root root     3273 Jun 18 05:50  fixup_cd.dat
-rwxr-xr-x. 1 root root     7370 Jun 18 05:50  fixup.dat
-rwxr-xr-x. 1 root root    10335 Jun 18 05:50  fixup_db.dat
-rwxr-xr-x. 1 root root    10337 Jun 18 05:50  fixup_x.dat
-rwxr-xr-x. 1 root root 22783778 Jun 18 06:00  initramfs_2712
-rwxr-xr-x. 1 root root 22788459 Jun 18 06:00  initramfs8
-rwxr-xr-x. 1 root root      145 Jun 18 06:01  issue.txt
-rwxr-xr-x. 1 root root 10172022 Jun 18 05:50  kernel_2712.img
-rwxr-xr-x. 1 root root 10168774 Jun 18 05:50  kernel8.img
-rwxr-xr-x. 1 root root     1594 Jun 18 05:50  LICENCE.broadcom
-rwxr-xr-x. 1 root root       38 Aug 18 19:21  meta-data
-rwxr-xr-x. 1 root root      320 Aug 18 19:21  network-config
drwxr-xr-x. 2 root root    32256 Jun 18 05:50  overlays
-rwxr-xr-x. 1 root root   851740 Jun 18 05:50  start4cd.elf
-rwxr-xr-x. 1 root root  3804392 Jun 18 05:50  start4db.elf
-rwxr-xr-x. 1 root root  2305632 Jun 18 05:50  start4.elf
-rwxr-xr-x. 1 root root  3053352 Jun 18 05:50  start4x.elf
-rwxr-xr-x. 1 root root   851740 Jun 18 05:50  start_cd.elf
-rwxr-xr-x. 1 root root  4876232 Jun 18 05:50  start_db.elf
-rwxr-xr-x. 1 root root  3029888 Jun 18 05:50  start.elf
-rwxr-xr-x. 1 root root  3777000 Jun 18 05:50  start_x.elf
drwxr-xr-x. 2 root root      512 Aug 19 09:37 'System Volume Information'
-rwxr-xr-x. 1 root root      477 Aug 18 19:21  user-data
```

Verify that the following essential Raspberry Pi 4 binaries exist and are non-zero size:

* `start4.elf` and `fixup4.dat` (Pi 4 specific primary firmware)
* `config.txt` and `cmdline.txt`
* `kernel8.img` (or `kernel7l.img` for 32-bit)

*If `start4.elf` is missing or named `start.elf` without the `4`, the Pi 4 bootloader will fail immediately after one blink.*

- All the necessary files are in place


## Moving onto EEPROM Sanity check
Determine if the issue is with the SD card interface or the Pi 4 bootloader:

1. Remove the SD card completely from the Pi 4.
2. Power on the Pi 4 with only the USB-C power cable attached.
3. Observe the green ACT LED:
* **4 Blinks Pattern (Repeated):** The EEPROM bootloader is healthy and actively looking for boot media. The issue is strictly with the SD card formatting or card reader slot.
* **Solid OFF / Single Blink:** The Pi 4 EEPROM bootloader is corrupted.

If EEPROM is corrupted: Re-flash the SD card using Raspberry Pi Imager under Misc Utility Images → Raspberry Pi 4 EEPROM boot recovery, insert it into the Pi, and power on until the green LED blinks rapidly.

### Observation
- Single blink : Hence EEPROM bootloader is corrupted

### Recovery
- Instead of blindly relying on Raspberry PI Imager, lets dig deep into the recovery procedure.


# Deep dive into bootloader stages of Raspberry Pi 4B 
- BCM2711 : SOC of Raspberry Pi 4B

## Hardware boot stages
- Two hardware boot stages 
### Stage 0: Mask ROM 
- Hardcoded in BCM2711 sillicon
- Initilizes basic CPU state and attempts to load bootloader from 512KB EEPROM chip
### Stage 1: SPI EEPROM 
- Initialize RAM, PCIe, and allocate start4.elf on SDCard 


## The recovery
When the SPI EEPROM is corrupted, Stage 0 fails to validate the EEPROM payload. However, the Mask ROM fallback mechanism looks specifically for a file named `recovery.bin` at the root of a FAT16/FAT32 formatted SD card.

When `recovery.bin` is found, the Mask ROM loads it directly into SRAM and executes it. `recovery.bin` then reprogram/re-flashes the onboard SPI EEPROM using image files (`pieeprom.bin`) present on the card.

Following are the procedure to construct EEPROM recovery SD Card

#### Step 1: Clean the SD Card Boot Partition
Unmount the SD card partitions and format `/dev/mmcblk0p1` as FAT32, or clear its contents completely:
```bash
± |vivek/low_level_exploration U:1 ?:3 ✗| → sudo rm -rf /mnt/sdcard_boot/*
[sudo] password for vivekv: 
± |vivek/low_level_exploration U:1 ?:3 ✗| → ls -la /mnt/sdcard_boot/
total 4
drwxr-xr-x. 2 root root 4096 Jan  1  1970 .
drwxr-xr-x. 1 root root   34 Aug 20 07:28 ..
```


#### Step 2: Fetch Official Raspberry Pi EEPROM Binaries

Download the latest recovery payload directly from the official [`raspberrypi/rpi-eeprom`](https://github.com/raspberrypi/rpi-eeprom) firmware repository.










### How Pi 4 EEPROM Recovery Works Under the Hood

The BCM2711 SoC on the Raspberry Pi 4 does not boot directly from the SD card. Instead, it relies on a two-stage hardware boot process:

1. **Mask ROM (Stage 0):** Hardcoded inside the BCM2711 silicon. At power-on, it initializes basic CPU state and attempts to load the bootloader from the onboard 512 KB SPI EEPROM chip.
2. **SPI EEPROM Bootloader (Stage 1):** Contains the logic to initialize RAM, PCIe (for USB 3.0), and locate `start4.elf` on the SD card/USB.

When the SPI EEPROM is corrupted, Stage 0 fails to validate the EEPROM payload. However, the Mask ROM fallback mechanism looks specifically for a file named `recovery.bin` at the root of a FAT16/FAT32 formatted SD card.

When `recovery.bin` is found, the Mask ROM loads it directly into SRAM and executes it. `recovery.bin` then reprogram/re-flashes the onboard SPI EEPROM using image files (`pieeprom.bin`) present on the card.

---

### Manual EEPROM Recovery Procedure

You can manually construct the EEPROM recovery SD card using your Linux workstation.

#### Step 1: Clean the SD Card Boot Partition

Unmount the SD card partitions and format `/dev/mmcblk0p1` as FAT32, or clear its contents completely:

```bash
sudo rm -rf /mnt/sdcard_boot/*

```

#### Step 2: Fetch Official Raspberry Pi EEPROM Binaries

Download the latest recovery payload directly from the official [`raspberrypi/rpi-eeprom`](https://github.com/raspberrypi/rpi-eeprom) firmware repository.

```bash
# Create a temporary working directory
mkdir -p /tmp/rpi_recovery && cd /tmp/rpi_recovery

# Fetch the latest stable recovery release archive from GitHub
wget https://github.com/raspberrypi/rpi-eeprom/raw/master/firmware-2711/release-notes.md
# Download the latest stable pieeprom binary and recovery bootloader
wget https://github.com/raspberrypi/rpi-eeprom/raw/master/firmware-2711/stable/recovery.bin
wget https://github.com/raspberrypi/rpi-eeprom/raw/master/firmware-2711/stable/pieeprom-2024-01-24.bin -O pieeprom.bin
wget https://github.com/raspberrypi/rpi-eeprom/raw/master/firmware-2711/stable/vl805-000138a2.bin -O vl805.bin

```

*Note: `vl805.bin` is the SPI firmware for the VIA VL805 USB 3.0 PCIe host controller integrated into the Pi 4.*

#### Step 3: Generate the SHA-256 Signature

`recovery.bin` verifies `pieeprom.bin` before writing to flash. Generate `pieeprom.sig` using `sha256sum`:

```bash
sha256sum pieeprom.bin | awk '{print $1}' > pieeprom.sig

```

#### Step 4: Populate the Boot Partition

Copy the generated binaries and signature to `/mnt/sdcard_boot/`:

```bash
sudo cp recovery.bin pieeprom.bin pieeprom.sig vl805.bin /mnt/sdcard_boot/
sync
sudo umount /mnt/sdcard_boot

```

---

### Execution & Hardware Verification

1. **Insert the prepared SD card** into the Raspberry Pi 4.
2. **Disconnect all peripherals** (no USB drives, ethernet, etc.), keeping only the USB-C power cable plugged in.
3. **Observe the Green ACT LED state progression:**
* **Initial Power On:** Solid or erratic blinking while `recovery.bin` executes from SRAM.
* **Flashing in Progress:** Steady, rhythmic flashing as SPI EEPROM registers are written.
* **Success:** Green ACT LED blinks **rapidly and continuously** (approx. 4 Hz / pattern with short pauses) and stays in an infinite loop.


4. **Power off the Pi 4** once the rapid blinking pattern is established.
5. **Re-flash your original OS image** onto the SD card. The SPI EEPROM is now restored, and the Pi 4 will proceed past Stage 0 and parse `start4.elf`.