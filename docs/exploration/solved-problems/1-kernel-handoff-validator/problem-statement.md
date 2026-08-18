## Problem Title: ARMv8 SoC Boot Stage & Kernel Handoff Validator

**Difficulty:** Hard

**Category:** Systems Programming / Firmware Architecture / Memory & Bitwise Manipulation


---

### Problem Description

In an ARM Cortex-A System-on-Chip (SoC) bringup pipeline, the hardware moves through multiple execution stages—from initial cold reset (BootROM/BL1) to early kernel entry (`head.S`).

You are tasked with building a boot-sequence validator engine for an ARMv8/v9 emulator. The engine must process a list of **boot stage descriptors**, translate local bus addresses into CPU physical memory space using **Flattened Device Tree (FDT) bus ranges**, verify **security/execution levels**, ensure **memory safety (no overlapping execution regions)**, and validate the **final CPU state against the Linux kernel handoff contract (`head.S`)**.

---

### Input Specifications

#### Data Structures

```c
// --- C Definition ---
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char stage_name[8];       // "BL1", "BL2", "BL31", "BL32", "BL33", "KERNEL"
    uint32_t execution_level; // 1 = EL1, 2 = EL2, 3 = EL3
    bool is_secure;           // true = Secure World, false = Non-Secure World
    uint64_t load_address;    // Base memory address
    uint64_t image_size;      // Size in bytes
    bool is_in_sram;          // true = On-chip SRAM, false = System DRAM
} BootStage;

typedef struct {
    uint64_t x[4];            // CPU General Purpose Registers x0, x1, x2, x3
    uint32_t sctlr;           // System Control Register (SCTLR_EL1/EL2)
    uint8_t daif;             // Interrupt mask bits: Bit 3:D, Bit 2:A, Bit 1:I, Bit 0:F
    uint32_t dtb_magic;       // Header magic value at memory address pointed to by x0
} CpuState;

typedef struct {
    uint64_t child_base;      // Child bus base physical address
    uint64_t parent_base;     // Parent (CPU physical) base address
    uint64_t size;            // Window size in bytes
} BusRange;

```

---

### Validation Rules

Your function must evaluate the parameters and return `true` if **ALL** the following system invariants pass, and write the calculated CPU physical address of the Device Tree Block into `out_dtb_phys_addr`. Otherwise, return `false`.

#### 1. Stage Sequence & Architectural Privilege Invariants

* **Chronological Order:** Stage execution order must follow `BL1` $\rightarrow$ `BL2` $\rightarrow$ `BL31` $\rightarrow$ `BL32` (Optional) $\rightarrow$ `BL33` $\rightarrow$ `KERNEL`.
* **Execution Levels & Security Rules:**
* **`BL1`**: Must execute in **Secure EL3** from **SRAM**.
* **`BL2`**: Must execute in **Secure EL1 or EL3** from **SRAM**.
* **`BL31`**: Must execute in **Secure EL3** (SRAM or DRAM).
* **`BL32`** (Optional OP-TEE): Must execute in **Secure EL1** from **DRAM**.
* **`BL33`** (U-Boot/Barebox): Must execute in **Non-Secure EL1 or EL2** from **DRAM**.
* **`KERNEL`**: Must execute in **Non-Secure EL1 or EL2** from **DRAM**.



#### 2. Memory Isolation & Bounds Checks

* **SRAM Bounds:** On-chip SRAM addresses span `[0x00000000, 0x00100000)` (1 MB). Any stage marked `is_in_sram == true` must fit entirely inside this range.
* **DRAM Bounds:** DRAM starts at `0x80000000`. Any stage marked `is_in_sram == false` must have `load_address >= 0x80000000`.
* **Non-Overlapping Memory:** No two stages may overlap in physical memory space.

#### 3. Device Tree Address Translation (`ranges`)

* You are given a raw DTB bus address (`dtb_bus_addr`). Translate it to CPU physical address using the `BusRange` mapping table:

$$\text{CPU Phys Addr} = \text{parent\_base} + (\text{dtb\_bus\_addr} - \text{child\_base})$$


* If `dtb_bus_addr` does not lie inside any range `[child_base, child_base + size)`, assume `dtb_bus_addr` is already a 1:1 mapped CPU physical address.

#### 4. Kernel Entry Contract (`head.S`) Invariants

* **`x0` Register:** `final_cpu_state->x[0]` MUST match the translated CPU physical address of the DTB.
* **`x1`, `x2`, `x3` Registers:** MUST be `0x0`.
* **`dtb_magic`:** MUST equal `0x0D00DFEED` (Flattened Device Tree Magic Number).
* **`SCTLR` Register:**
* Bit 0 (`MMU Enable`): MUST be `0` (MMU disabled).
* Bit 2 (`D-Cache Enable`): MUST be `0` (Data cache disabled).


* **`DAIF` PSTATE Interrupt Mask:** Lower 4 bits (`D, A, I, F`) MUST all be `1` (i.e., `daif & 0x0F == 0x0F`), indicating Debug, SError, IRQ, and FIQ exceptions are masked upon kernel entry.

---

### Example Test Cases

#### Example 1: Valid Boot Pipeline

```text
Inputs:
Stages: [
  { "BL1",    EL3, Secure=true,  addr=0x00000000, size=0x10000, is_sram=true  },
  { "BL2",    EL3, Secure=true,  addr=0x00020000, size=0x20000, is_sram=true  },
  { "BL31",   EL3, Secure=true,  addr=0x80000000, size=0x10000, is_sram=false },
  { "BL33",   EL2, Secure=false, addr=0x80200000, size=0x80000, is_sram=false },
  { "KERNEL", EL2, Secure=false, addr=0x80800000, size=0x200000,is_sram=false }
]
Bus Ranges: [ { child_base=0x10000000, parent_base=0x88000000, size=0x02000000 } ]
dtb_bus_addr: 0x10000100

CpuState:
  x[0]: 0x88000100
  x[1]: 0x0, x[2]: 0x0, x[3]: 0x0
  sctlr: 0x00000000 (MMU off, D-Cache off)
  daif: 0x0F (All exceptions masked)
  dtb_magic: 0xD00DFEED

Output: true (out_dtb_phys_addr set to 0x88000100)

```

#### Example 2: Invalid Handoff Contract (MMU Left Enabled)

```text
Inputs:
CpuState:
  x[0]: 0x88000100
  sctlr: 0x00000001 (Bit 0 set -> MMU Enabled!)

Output: false
Reason: Kernel contract violated! MMU must be disabled at head.S entry.

```

---

### Constraints

* `1 <= num_stages <= 6`
* `0 <= num_ranges <= 16`
* `0x00000000 <= image_size <= 0x40000000` (Max 1GB per stage)

---