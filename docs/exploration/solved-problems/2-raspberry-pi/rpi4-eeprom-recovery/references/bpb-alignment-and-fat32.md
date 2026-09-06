# FAT32 BPB Alignment & Geometry Reference

## The BPB Hidden Sectors Issue (`0x1C`)

When formatting a FAT32 volume on a partition (e.g. `/dev/mmcblk0p1` starting at sector `2048`), standard Linux utilities write the Volume Boot Record (VBR) at sector `2048`.

Inside the VBR's BIOS Parameter Block (BPB):
- **Offset `0x1C` (4 bytes, uint32_t):** `Hidden Sectors` field.
- **Specification Rule:** This field MUST contain the number of physical sectors preceding the start of the FAT volume (i.e. `2048`).

Standard Linux kernel drivers ignore BPB `Hidden Sectors` because they use LBA offsets from the MBR partition table directly. However, **low-level embedded Boot ROMs (such as BCM2711 Stage 0)** use the BPB `Hidden Sectors` value to calculate absolute sector reads for the FAT cluster table and root directory.

If `Hidden Sectors` is set to `0`:
- Boot ROM adds `0` instead of `2048` to FAT cluster offsets.
- Boot ROM reads garbage from sector `32` instead of sector `2080`.
- Result: Boot ROM fails to find `recovery.bin` and hangs on **Solid Green LED**.

---

## Correct Formatting Command for Recovery Volumes

To format a FAT32 partition so it is 100% compliant with embedded Boot ROMs:

```bash
sudo mkfs.fat -F 32 -s 8 -h 2048 -n "RECOVERY" /dev/mmcblk0p1
```

Where:
- `-F 32`: Force 32-bit FAT table.
- `-s 8`: 8 sectors per cluster (4096-byte clusters). Ensures total cluster count > 65,525.
- `-h 2048`: Explicitly set `Hidden Sectors` = 2048 in BPB.
- `-n "RECOVERY"`: Volume label.

---

## Inspection & Repair Python Snippet

The following Python script checks and repairs `Hidden Sectors` at offset `0x1C` on a partition and syncs the backup boot sector (Sector 6):

```python
import struct

device_path = "/dev/mmcblk0p1"
target_hidden_sectors = 2048

with open(device_path, "rb+") as f:
    # Read Sector 0 (VBR)
    f.seek(0)
    sector0 = bytearray(f.read(512))
    
    current_hidden = struct.unpack("<I", sector0[0x1C:0x20])[0]
    print(f"Current BPB Hidden Sectors: {current_hidden}")
    
    if current_hidden != target_hidden_sectors:
        print(f"Updating BPB Hidden Sectors to {target_hidden_sectors}...")
        struct.pack_into("<I", sector0, 0x1C, target_hidden_sectors)
        
        # Write Sector 0
        f.seek(0)
        f.write(sector0)
        
        # Write Backup Boot Sector (Sector 6)
        f.seek(6 * 512)
        f.write(sector0)
        f.flush()
        print("BPB and Backup Boot Sector successfully updated!")
    else:
        print("BPB Hidden Sectors is already correct.")
```
