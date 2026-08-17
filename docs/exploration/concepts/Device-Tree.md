# Device Tree Architecture & Runtime Subsystem

Device Tree (DT) provides a hardware description mechanism that separates low-level board layout details from compiled Linux kernel images. It enables a single compiled binary kernel (`Image`) to support diverse System-on-Chip (SoC) architectures.

---

## 1. Flattened Device Tree (FDT) Binary Structure

When `.dts` source files are compiled using `dtc` (Device Tree Compiler), they produce a binary `.dtb` file structured according to the Flattened Device Tree Specification:

```
+-------------------------------------------------------------+
| FDT HEADER (struct fdt_header)                              |
|   magic: 0xd00dfeed                                         |
|   totalsize, off_dt_struct, off_dt_strings, off_mem_rsvmap |
+-------------------------------------------------------------+
| MEMORY RESERVED MAP (struct fdt_reserve_entry[])            |
|   Physical memory regions reserved for firmware/Secure World|
+-------------------------------------------------------------+
| STRUCTURE BLOCK (Tokens & Nodes)                            |
|   FDT_BEGIN_NODE, FDT_PROP, FDT_END_NODE, FDT_END           |
+-------------------------------------------------------------+
| STRINGS BLOCK                                               |
|   Null-terminated property name strings (e.g. "compatible") |
+-------------------------------------------------------------+
```

### FDT Header Field Reference

| Field | Type | Description |
| --- | --- | --- |
| `magic` | `uint32_t` | Constant magic value `0xd00dfeed` (Big Endian). |
| `totalsize` | `uint32_t` | Total size of the DTB binary in bytes. |
| `off_dt_struct` | `uint32_t` | Byte offset to Structure Block. |
| `off_dt_strings` | `uint32_t` | Byte offset to Strings Block. |
| `off_mem_rsvmap` | `uint32_t` | Byte offset to Memory Reserve Map. |
| `version` | `uint32_t` | FDT specification version (typically `17`). |
| `boot_cpuid_phys` | `uint32_t` | Physical CPU ID (`MPIDR`) of the primary boot core. |

---

## 2. Address Translation & Ranges Mapping

Address decoding across internal buses (e.g., PCIe, local SoC bus, peripheral bus) uses `#address-cells`, `#size-cells`, and `ranges`.

```
          Parent Bus Address Space (e.g., CPU Physical Bus: 64-bit)
   +------------------------------------------------------------------------+
   | 0x0000_0000_FE00_0000 -----------------------> 0x0000_0000_FE00_FFFF   |
   +------------------------------------------------------------------------+
                                 ▲
                                 │ ranges = <0x0 0xfe000000 0x10000>;
                                 ▼
   +------------------------------------------------------------------------+
   | 0x0000_0000 ---------------------------------> 0x0000_FFFF             |
   +------------------------------------------------------------------------+
          Child Bus Address Space (e.g., Peripheral Bus: 32-bit)
```

### Address Parsing Matrix

| Property | Context Scope | Bit-width Calculation | Example Value | Meaning |
| --- | --- | --- | --- | --- |
| `#address-cells` | Parent Node | `value * 32` bits | `<2>` | Address fields in child nodes take 2 32-bit cells (64-bit address). |
| `#size-cells` | Parent Node | `value * 32` bits | `<1>` | Size fields in child nodes take 1 32-bit cell (32-bit length). |
| `reg` | Child Node | Dynamic based on parent cells | `<0x0 0xFE215000 0x0 0x1000>` | Base MMIO address = `0xFE215000`, Size = `0x1000` (4KB). |
| `ranges` | Child Bus | Mapping tuple | `<child_addr parent_addr size>` | Maps local child bus offset to parent CPU physical memory space. |

---

## 3. Peripheral Properties & Binding Syntax

| Property | Cell Format / Syntax | Driver Usage / Subsystem Mapping |
| --- | --- | --- |
| `compatible` | `"vendor,device-model", "sys,generic"` | Matched against Linux kernel `struct of_device_id` tables. |
| `interrupts` | `<type index trigger_type>` | `GIC_SPI` (0) or `GIC_PPI` (1), INTID offset, trigger flags (Edge/Level). |
| `interrupt-parent` | `<&phandle>` | Pointer to the controlling `interrupt-controller` DT node. |
| `clocks` | `<&clk_controller CLK_ID>` | Pointer to clock provider node and specifier tuple. |
| `status` | `"okay"` / `"disabled"` | Determines whether kernel instantiates device node during probe. |

---

## 4. Kernel Driver Matching & Probe Lifecycle

```
 DT Compiler (.dts -> .dtb)
         │
         ▼
 Loaded into Memory by BL33 (Address passed in x0)
         │
         ▼
 [early_init_dt_scan()] Parses memory ranges & command line (bootargs)
         │
         ▼
 [of_platform_populate()] Iterates nodes, instantiates 'struct platform_device'
         │
         ▼
 Linux Driver Core matches 'compatible' string with registered drivers
         │
         ▼
 Invokes Driver's probe() function -> Allocates MMIO, maps IRQs, initializes HW
```

### Code Match Table Pattern

```c
static const struct of_device_id my_driver_dt_match[] = {
    { .compatible = "arm,pl011", .data = &pl011_data },
    { .compatible = "raspberrypi,bcm2835-aux-uart", },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, my_driver_dt_match);

static struct platform_driver my_driver = {
    .probe  = my_driver_probe,
    .remove = my_driver_remove,
    .driver = {
        .name = "my_uart_driver",
        .of_match_table = my_driver_dt_match,
    },
};
```
