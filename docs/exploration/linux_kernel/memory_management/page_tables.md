#  Page Table Architecture & Multi-Level Translation Mechanics

A flat array mapping every 4KB page across a 64-bit virtual address space would consume 512 gigabytes of physical RAM per process just to store translation descriptors. Linux solves this by organizing page tables into a sparse, multi-level tree hierarchy (PGD $\to$ PUD $\to$ PMD $\to$ PTE) where lookup tables are allocated dynamically only for virtual address ranges currently in use.

---

### The Multi-Level Tree Hierarchy

Instead of maintaining a single massive dictionary, Linux splits the Virtual Address ($VA$) into distinct index chunks. Each chunk acts as an offset into a 4KB node of the page tree.

```
                  TTBRx_EL1 (Base Register)
                              │
                              ▼
                +--------------------------+
                |  Page Global Directory   | (PGD - Level 0)
                +-------------+------------+
                              │
                              ▼
                +--------------------------+
                |   Page Upper Directory   | (PUD - Level 1)
                +-------------+------------+
                              │
                              ▼
                +--------------------------+
                |  Page Middle Directory   | (PMD - Level 2)
                +-------------+------------+
                              │
                              ▼
                +--------------------------+
                |     Page Table Entry     | (PTE - Level 3)
                +-------------+------------+
                              │
                              ▼
                 [ Physical RAM Page Frame ]

```

* **Page Global Directory (PGD - Level 0):** The root of the translation tree. The CPU's Translation Table Base Register (`TTBR0_EL1` for user space, `TTBR1_EL1` for kernel space) holds the physical address of the active `PGD`.
* **Page Upper Directory (PUD - Level 1):** Level 1 branch node. Points to a PMD table or maps a **1GB Huge Page** directly (bypassing levels 2 and 3).
* **Page Middle Directory (PMD - Level 2):** Level 2 branch node. Points to a PTE table or maps a **2MB Block Page** directly.
* **Page Table Entry (PTE - Level 3):** Level 3 leaf node. Stores the physical frame address and permission attributes for an individual 4KB page frame.

---

### Address Bit Slicing & Hardware Lookup Walk

On ARM64 with a 48-bit Virtual Address space and a 4KB page granule, the MMU slices a 48-bit address into four 9-bit table indices and one 12-bit page offset.

$$\text{VA [47:0]} = \underbrace{\text{PGD}}_{\text{9 bits}} \;\Vert\; \underbrace{\text{PUD}}_{\text{9 bits}} \;\Vert\; \underbrace{\text{PMD}}_{\text{9 bits}} \;\Vert\; \underbrace{\text{PTE}}_{\text{9 bits}} \;\Vert\; \underbrace{\text{Offset [11:0]}}_{\text{12 bits}}$$

Each 9-bit field selects one of $2^9 = 512$ entries within a 4KB table page ($512 \text{ entries} \times 8 \text{ bytes/descriptor} = 4096 \text{ bytes}$).

#### Step-by-Step Hardware Page Table Walk

1. **Fetch PGD Base:** MMU reads the base physical address of the `PGD` from `TTBRx_EL1`.
2. **Level 0 Indexing:** MMU uses `VA[47:39]` as an index into `PGD` to read the 64-bit descriptor. This descriptor gives the physical base address of the target `PUD`.
3. **Level 1 Indexing:** MMU uses `VA[38:30]` as an index into `PUD` to get the physical base address of the target `PMD`.
4. **Level 2 Indexing:** MMU uses `VA[29:21]` as an index into `PMD` to get the physical base address of the target `PTE`.
5. **Level 3 Indexing:** MMU uses `VA[20:12]` as an index into `PTE` to extract the **Physical Frame Number (PFN)**.
6. **Physical Address Resolution:** MMU appends `VA[11:0]` directly to the PFN to produce the final target DRAM physical address:

$$\text{Physical Address} = (\text{PFN} \ll 12) \mid \text{Offset [11:0]}$$

If any intermediate entry has its `Valid` bit cleared to `0`, the MMU aborts the walk immediately and raises a synchronous **Translation Fault** to the CPU. Early boot code (`head.S`) relies on this dynamic creation of early tables (`init_pg_dir`) in physical RAM before turning on the MMU.

---

### Page Table Entry (PTE) Hardware Descriptor Format

Each 64-bit entry in a leaf page table contains hardware control flags and the destination physical page frame address.

```
63       54 53  52 51          48 47                       12 11   10  9 8  7  6 5    2 1 0
+-----------+---+---+--------------+---------------------------+-------+--+--+--+----+---+---+
| Upper Attr| XN|PXN|  Reserved    | Output Physical Address   | Lower |AF|SH|AP| NS |1  |V  |
|           |   |   |              |       [47:12]             | Attr  |  |  |  |    |   |   |
+-----------+---+---+--------------+---------------------------+-------+--+--+--+----+---+---+

```

| Bit / Field | Name | Hardware Function |
| --- | --- | --- |
| **Bit 0** | `V` (Valid) | `1` = Entry active. `0` = Unmapped address (triggers Translation Fault).|
| **Bit 1** | Table / Page | `1` = Points to a 4KB page or next-level table. `0` = Block mapping (2MB/1GB). |
| **Bits [5:2]** | `AttrIndx` | Index into Memory Attribute Indirection Register (`MAIR_EL1`) defining cacheability (Normal Memory vs. Device MMIO). |
| **Bits [7:6]** | `AP` | **Access Permissions:** `00` (RW EL1), `01` (RW EL1/EL0), `10` (RO EL1), `11` (RO EL1/EL0). |
| **Bits [9:8]** | `SH` | **Shareability:** `00` (Non-shareable), `10` (Outer Shareable), `11` (Inner Shareable for SMP cores). |
| **Bit 10** | `AF` | **Access Flag:** Set by HW/SW when read/written; used by kernel reclaim engines to detect stale pages. |
| **Bits** | `Output Address` | High bits of the target physical DRAM address. |
| **Bit 53** | `PXN` | **Privileged Execute-Never:** Blocks kernel (`EL1`) from executing code in user pages. |
| **Bit 54** | `UXN / XN` | **Unprivileged Execute-Never:** Blocks execution in stack/data pages (W^X security model). |

---

### Quantifying Memory Savings: Flat Table vs. 4-Level Tree

Consider a process with a 48-bit address space ($256 \text{ TB}$) that only allocates **8 Megabytes** of RAM split into two distinct regions: 4MB for code at the bottom of memory, and 4MB for stack at the top of memory.

#### 1. Flat Array Approach

* Total Pages in 48-bit Space: $\frac{2^{48}}{4096} = 68,719,476,736 \text{ pages}$
* Overhead: $68,719,476,736 \times 8 \text{ bytes/entry} = \mathbf{512 \text{ GB}}$ of memory just to store translation descriptors, even if only 8MB of RAM is used!

#### 2. Multi-Level Tree Approach (Linux Sparse Allocation)

Because unallocated memory branches are simply represented as null descriptors in higher tables, only the following node pages are allocated in RAM:

| Table Level | Allocated Tables | Memory Overhead | Reason |
| --- | --- | --- | --- |
| **PGD** | 1 Page | 4 KB | Root node covering all 256 TB. |
| **PUD** | 2 Pages | 8 KB | 1 for low memory code, 1 for high memory stack. |
| **PMD** | 2 Pages | 8 KB | 1 for low memory code, 1 for high memory stack. |
| **PTE** | 4 Pages | 16 KB | Each PTE page maps 512 entries ($512 \times 4\text{KB} = 2\text{MB}$). Need 2 per 4MB region. |
| **Total Overhead** | **9 Pages** | **36 KB** | Dynamic tree allocation saves over **99.9999%** of table memory. |


```text
========================================================================================
 TOP OF HIERARCHY: HARDWARE ROOT REGISTER
========================================================================================
 TTBRx_EL1 Register ---> Holds Physical Address of active PGD
                         (User space uses TTBR0_EL1, Kernel uses TTBR1_EL1)

========================================================================================
 LEVEL 0: PGD (Page Global Directory)
========================================================================================
 [ PGD Table Node: 1 Page in RAM = 4096 bytes = 512 Descriptors (8 bytes each) ]
 +------------------------------------------------------------------------------------+
 | Entry 0   | Entry 1 | ... | Entry 256 (Selected by VA[47:39]) | ... | Entry 511   |
 +----------------------------------+-------------------------------------------------+
                                    |
                                    +----> Points to Physical Base Address of 1 PUD Table
                                           (1 PGD Entry covers 512 GB of Virtual Memory)

========================================================================================
 LEVEL 1: PUD (Page Upper Directory)
========================================================================================
 [ PUD Table Node: 1 Page in RAM = 4096 bytes = 512 Descriptors (8 bytes each) ]
 +------------------------------------------------------------------------------------+
 | Entry 0   | ... | Entry 32 (Selected by VA[38:30]) | ...               | Entry 511   |
 +---------------------------+--------------------------------------------------------+
                             |
                             +----> Points to Physical Base Address of 1 PMD Table
                                    (1 PUD Entry covers 1 GB of Virtual Memory)
                                    *(Or maps a 1 GB Huge Page directly)*

========================================================================================
 LEVEL 2: PMD (Page Middle Directory)
========================================================================================
 [ PMD Table Node: 1 Page in RAM = 4096 bytes = 512 Descriptors (8 bytes each) ][cite: 1]
 +------------------------------------------------------------------------------------+
 | Entry 0   | Entry 1 | ... | Entry 4 (Selected by VA[29:21]) | ...     | Entry 511   |
 +-------------------------------+----------------------------------------------------+
                                 |
                                 +----> Points to Physical Base Address of 1 PTE Table[cite: 1]
                                        (1 PMD Entry covers 2 MB of Virtual Memory)[cite: 1]
                                        *(Or maps a 2 MB Block Page directly)*[cite: 1]

========================================================================================
 LEVEL 3: PTE (Page Table Entry) — LEAF NODE
========================================================================================
 [ PTE Table Node: 1 Page in RAM = 4096 bytes = 512 Descriptors (8 bytes each) ][cite: 1]
 +------------------------------------------------------------------------------------+
 | Entry 0   | ... | Entry 171 (Selected by VA[20:12]) | ...             | Entry 511   |
 +---------------------------+--------------------------------------------------------+
                             |
                             +----> Points to Base Physical Frame Number (PFN) in RAM[cite: 1]
                                    (1 PTE Entry covers a single 4 KB Page Frame)[cite: 1]

========================================================================================
 BOTTOM: VIRTUAL ADDRESS BIT-SLICING (Input: 0xffff800080ab07cc)[cite: 1]
========================================================================================
 +-------------------+-------------------+-------------------+-------------------+----------------------+
 | PGD Index (9 bits)| PUD Index (9 bits)| PMD Index (9 bits)| PTE Index (9 bits)| Page Offset (12 bits)|
 |    Bits [47:39]   |    Bits [38:30]   |    Bits [29:21]   |    Bits [20:12]   |     Bits [11:0]      |
 |     = 256 (0x100) |      = 32 (0x20)  |       = 4 (0x4)   |     = 171 (0xAB)  |      = 0x7CC         |
 +---------+---------+---------+---------+---------+---------+---------+---------+----------+-----------+
           |                   |                   |                   |                    |
           v                   v                   v                   v                    v
       Selects Entry       Selects Entry       Selects Entry       Selects Entry       Added to Physical
      in PGD Table        in PUD Table        in PMD Table        in PTE Table        Frame Base Address[cite: 1]

========================================================================================
 DESTINATION: PHYSICAL DRAM PAGE FRAME
========================================================================================
 [ Physical Page Frame in RAM (4 KB) ][cite: 1]
 +------------------------------------------------------------------------------------+
 | Offset 0x000 ... | Target Byte Offset 0x7CC | ...                 | Offset 0xFFF   |
 +------------------------------------------------------------------------------------+
```

Each level in the ARM64 translation hierarchy multiplies virtual memory coverage by a factor of 512, scaling from a single 4 KB physical DRAM page to the entire 256 TB virtual address space.

```text
  PGD Table (512 entries) ──► 256 TB  [Entire 48-bit Virtual Address Space]
       │
       ├──► PUD Table (512 entries) ──► 512 GB  [Per PGD Entry]
             │
             ├──► PMD Table (512 entries) ──► 1 GB  [Per PUD Entry / Huge Page]
                   │
                   ├──► PTE Table (512 entries) ──► 2 MB  [Per PMD Entry / Block Page]
                         │
                         └──► 1 PTE Entry ──► 4 KB  [Single Physical DRAM Page]

```

---

| Level | Granule Unit | Calculation | Virtual Space Mapped |
| --- | --- | --- | --- |
| **1 PTE Entry** | Leaf Entry | $1 \times 4 \text{ KB}$ | **4 KB**<br> |
| **1 PTE Table** | 512 PTEs | $512 \times 4 \text{ KB}$ | **2 MB**<br> |
| **1 PMD Table** | 512 PMDs | $512 \times 2 \text{ MB}$ | **1 GB**<br> |
| **1 PUD Table** | 512 PUDs | $512 \times 1 \text{ GB}$ | **512 GB**<br> |
| **1 PGD Table** | 512 PGDs | $512 \times 512 \text{ GB}$ | **256 TB**<br> |

**Core Takeaway**

Each step up the tree adds 9 bits of address indexing ($2^9 = 512$), allowing a compact 4 KB table page at the top level to manage a massive virtual address space.