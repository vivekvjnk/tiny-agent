# Linux Kernel Decompression Architecture

Kernel decompression is the early boot process where a compressed operating system image uncompresses itself or is uncompressed by a bootloader into RAM before kernel initialization (`start_kernel()`).

---

## 1. Architectural Strategy Comparison

```
+-----------------------------------------------------------------------------------+
| ARCHITECTURE   | KERNEL IMAGE FORMAT | DECOMPRESSION IMPLEMENTATION               |
+----------------+---------------------+--------------------------------------------+
| ARM32          | zImage              | Self-Decompressing Kernel Stub             |
|                |                     | (Extracts piggy.gz/lz4/zstd in-place)      |
+----------------+---------------------+--------------------------------------------+
| AArch64        | Image (Uncompressed)| Bootloader / Firmware Level Decompression  |
| (ARM64)        | or Image.gz / EFI   | (U-Boot booti / FIT image / EFI stub)      |
+----------------+---------------------+--------------------------------------------+
| x86 / x86_64   | bzImage             | Self-Decompressing Setup Stub              |
|                |                     | (Extracts startup_64 / vmlinux)            |
+----------------+---------------------+--------------------------------------------+
```

---

## 2. AArch32 Self-Decompressing (`zImage`) Flow & Memory Layout

In 32-bit ARM architectures, `zImage` contains a prepended decompressor head (`arch/arm/boot/compressed/head.S`) attached to the compressed payload (`piggy.gz` / `piggy.lz4`).

```
 DRAM Memory Address Space
  ▲
  │
  ├─ [Decompressed Kernel Target Area] <─── Decompressor extracts kernel here
  │    (e.g., DRAM_BASE + 0x8000)
  │
  ├─ [zImage Execution Stub] ───────────┐ Executed directly by bootloader
  │    • head.S                         │ Sets up page tables, enables MMU/Cache
  │    • decompress.c                   │
  │    • compressed payload (piggy.o) ──┘
  │
  └─ [Malloc / Stack Area] ────────────── Allocates decompression buffer workspace
```

### Self-Decompression Sequence

1. **`head.S` Execution:** Saves boot parameters passed in registers (`r0` = 0, `r1` = Machine ID, `r2` = DTB pointer).
2. **Cache Initialization:** Enables Instruction/Data Caches and MMU temporarily to accelerate decompression speed by up to 10x.
3. **Relocation Check:** Compares execution address against target address. If overwrite collision is detected, `zImage` relocates itself higher in DRAM.
4. **`decompress_kernel()` Call:** Calls target algorithm decompressor (`decompress.c` wrapper calling `gunzip`, `unlz4`, or `unzstd`).
5. **Cache Flush & Clean:** Disables D-Cache/MMU, flushes I-Cache to maintain coherency.
6. **Kernel Jump:** Branches to decompressed kernel entry point (`stext`).

---

## 3. AArch64 (ARM64) Decompression Strategy

The 64-bit Arm architecture (`AArch64`) removed kernel self-decompression from `arch/arm64/boot/` to simplify entry invariants and kernel maintainability.

```
 +---------------------------------------------------------------------------------+
 | OPTION A: BOOTLOADER-BASED DECOMPRESSION (U-BOOT FIT IMAGE)                     |
 |                                                                                 |
 |  • U-Boot loads 'Image.gz' from eMMC / SPI Flash                                |
 |  • U-Boot decompresses payload directly into DRAM target address                 |
 |  • Executes 'booti' command -> Jumps to uncompressed 'Image' entry               |
 +---------------------------------------------------------------------------------+

 +---------------------------------------------------------------------------------+
 | OPTION B: UEFI EFI STUB DECOMPRESSION (vmlinuz / ZBOOT)                         |
 |                                                                                 |
 |  • Linux kernel compiled with CONFIG_EFI_ZBOOT                                  |
 |  • Small EFI stub wrapper parses PE/COFF header and decompresses embedded       |
 |    Image before jumping to Kernel head.S                                        |
 +---------------------------------------------------------------------------------+
```

---

## 4. Compression Algorithm Comparison Matrix

| Algorithm | Kernel Config Flag | Compression Ratio | Decompression Speed | Boot Time Impact | Memory Overhead |
| --- | --- | --- | --- | --- | --- |
| **GZIP** | `CONFIG_KERNEL_GZIP` | Moderate (Baseline) | Moderate | Baseline | Very Low (~64KB) |
| **BZIP2** | `CONFIG_KERNEL_BZIP2` | High | Slow | Worst | High (~1MB) |
| **LZMA** | `CONFIG_KERNEL_LZMA` | Best (Smallest size) | Slow | Slow | High (~12MB) |
| **LZO** | `CONFIG_KERNEL_LZO` | Lower | Fast | Good | Low (~64KB) |
| **LZ4** | `CONFIG_KERNEL_LZ4` | Lower | Very Fast | Fast | Very Low (~64KB) |
| **ZSTD** | `CONFIG_KERNEL_ZSTD` | High (Near LZMA) | Extremely Fast (Near LZ4) | **Optimal Best** | Low (~1MB) |
