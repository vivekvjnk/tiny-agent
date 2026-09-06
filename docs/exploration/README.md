# Low-Level Embedded Knowledge Network & Engineering Exploration Master Index

Welcome to the **Embedded Systems & Platform Architecture Knowledge Network**. This master index provides a highly coherent, interconnected map of low-level embedded software, SoC bringup, silicon hardware microarchitectures, Linux kernel internals, systems C/C++ programming, and compiler optimization techniques.

---

## 1. Master Knowledge Network Architecture Map

```
                               ┌────────────────────────────────────────────────────────┐
                               │       SYSTEM ARCHITECTURE KNOWLEDGE NETWORK            │
                               └───────────────────────────┬────────────────────────────┘
                                                           │
        ┌──────────────────────────────────────────────────┼──────────────────────────────────────────────────┐
        │                                                  │                                                  │
        ▼                                                  ▼                                                  ▼
+─────────────────────────────────+        +─────────────────────────────────+        +─────────────────────────────────+
| 1. SOC BOOT & SILICON BRINGUP   |        | 2. CORE COMPUTING & ACCELERATION|        | 3. LINUX KERNEL INTERNALS       |
|   • BootROM, BL1, BL2, BL31, BL33|        |   • Von Neumann Bus & MMIO vs PMIO  |        |   • Virtual Memory & 4-Level Page   |
|   • Device Tree, GIC, Timers    |        |   • DDR PHY Calibration, PCIe BARs  |        |     Tables (PGD -> PUD -> PMD ->  |
|   • Kernel Decompression & Entry|        |   • GPUs, Systolic Arrays (TPUs)    |        |     PTE Walk), Process Layout     |
|   • Barebox & Boot Troubleshooting|        |   • Store Buffers & Memory Barriers |        |   • Allocators, ASLR, QEMU Headless|
|                                 |        |                                 |        |                                 |
| ├─► Master Hub:                 |        | ├─► Master Hub:                 |        | ├─► Master Hub:                 |
| [Cortex-A Boot Hub](concepts/HUB_ARM-Cortex-A-Boot-Process.md)| | [Core Computing Hub](concepts/HUB_Core_Computing.md)| | [Linux Kernel Hub](linux_kernel/hub_kernel.md)|
+─────────────────────────────────+        +─────────────────────────────────+        +─────────────────────────────────+
        │                                                  │                                                  │
        ├──────────────────────────────────────────────────┼──────────────────────────────────────────────────┘
        │                                                  │
        ▼                                                  ▼
+─────────────────────────────────+        +─────────────────────────────────+
| 4. SYSTEMS C/C++ & CONCURRENCY  |        | 5. COMPILER OPTIMIZATIONS & IR  |
|   • C Memory Model, Storage Duration|        |   • Algorithmic Strength Reduction  |
|   • Qualifiers (`volatile`, `const`)|        |     & Constant Synthesis          |
|   • Pointer Declaration Clockwise   |        |   • Bit Twiddling Hacks &           |
|     Spiral Parsing Algorithm    |        |     Branchless Programming          |
|   • Lock-Free RCU & SPSC Ring Buffers|       |   • Hardware Gate & ISA Lowering    |
|                                 |        |                                 |
| ├─► Master Hub:                 |        | ├─► Master Hub:                 |
| [Systems C/C++ Hub](c_c++/hub_concepts.md)| | [Compiler Optimizations Hub](compiler_optimization_techniques/hub_compiler_optimization_concepts.md)|
+─────────────────────────────────+        +─────────────────────────────────+
        │                                                  │
        └──────────────────────────────────────────────────┴──────────────────────────────────────────────────┐
                                                           │                                                  │
                                                           ▼                                                  ▼
                                           +─────────────────────────────────+        +─────────────────────────────────+
                                           | 6. SOLVED PROBLEMS & BRINGUP    |        | 7. CURRICULUM EXPLORATION PLAN  |
                                           |   • ARMv8 Kernel Handoff Validator  |        |   • 7-Day Hands-on Sprint Roadmap   |
                                           |   • Raspberry Pi 4 SPI EEPROM       |        |   • TPU BSP Driver, DMA Pinning &   |
                                           |     Recovery & FAT32 Alignment      |        |     Doorbell Handshaking            |
                                           |                                 |        |                                 |
                                           | ├─► Master Hub:                 |        | ├─► Roadmap Index:              |
                                           | [Solved Problems Hub](solved-problems/README.md)| | [7-Day Exploration Plan](exploration_plan.md)|
                                           +─────────────────────────────────+        +─────────────────────────────────+
```

---

## 2. Directory Navigation & Master Hub Index

| Knowledge Domain | Primary Scope & Focus | Master Architecture Hub Link | Deep Dive References |
| --- | --- | --- | --- |
| **1. SoC Boot Sequence & Silicon Bringup** | ARMv8/ARMv9 boot chain (POR $\rightarrow$ BootROM $\rightarrow$ TF-A BL1/BL2/BL31 $\rightarrow$ BL33 $\rightarrow$ Kernel `head.S`), Device Tree, GIC, Timers, and Barebox. | **[SoC Boot Master Hub](concepts/HUB_ARM-Cortex-A-Boot-Process.md)** | [BL2](concepts/BL2.md), [TFA](concepts/TFA.md), [BL33](concepts/BL33.md), [EL Architecture](concepts/Exception-Levels-in-ARM.md), [Device Tree](concepts/Device-Tree.md), [GIC Spec](concepts/Generic-Interrupt-Controller.md), [Generic Timer](concepts/standard-arm-timer.md), [Kernel Decompression](concepts/Kernel-Decompression.md), [Barebox Tool](tools/Barebox.md), [Boot FAQ](questions/boot.md) |
| **2. Hardware Architecture & Acceleration** | Von Neumann system bus, MMIO vs PMIO, DDR PHY calibration, PCIe interconnects, GPU parallel compute, spatial/systolic array TPUs, store buffers, and barriers. | **[Core Computing Master Hub](concepts/HUB_Core_Computing.md)** | [System Bus](concepts/von-neumann-system-bus-and-io-architecture.md), [DDR PHY](concepts/Memory.md), [PCIe](concepts/pcie.md), [GPU Microarchitecture](concepts/gpu.md), [Spatial TPU](concepts/spacial-computing.md), [Fast Store](concepts/fast_store_delayed_drain.md), [Store-Load Reordering](concepts/store_load_reordering.md), [Release-Acquire](concepts/release_store_n_acquire_load.md), [TSC Timing](concepts/hpc_time_measurement.md) |
| **3. Linux Kernel Internals & OS** | Virtual memory, 4-level page table walks (`PGD`/`PUD`/`PMD`/`PTE`), process memory layout (`brk`/`sbrk`, `fork`, `execve`), physical allocators, ASLR/KASLR, and QEMU headless direct-kernel boot. | **[Linux Kernel Master Hub](linux_kernel/hub_kernel.md)** | [Page Tables Walk](linux_kernel/memory_management/page_tables.md), [Virtual & Physical Memory](linux_kernel/memory_management.md), [Kernel Debugging & Driver Model](linux_kernel/kernel_debugging.md), [ASLR Mechanics](linux_kernel/resources/aslr.md), [QEMU Direct-Kernel Boot](linux_kernel/resources/qemu/readme.md) |
| **4. Systems C/C++, Memory & Concurrency** | C memory model, type qualifiers (`const`, `volatile`, `restrict`, `static`), pointer declaration parsing, C++ templates/vectors, bit manipulation, and lock-free RCU / ring buffers. | **[Systems C/C++ Master Hub](c_c++/hub_concepts.md)** | [Pointers Hub](c_c++/pointers/hub_concepts.md), [C Memory Model](c_c++/general/memory_model_n_general_concepts.md), [Parsing Rule](c_c++/pointers/pointer_declaration_parsing.md), [C++ Templates](c_c++/cpp_concepts/templates.md), [C++ Vectors](c_c++/cpp_concepts/vectors.md), [Bit Manipulation](c_c++/code/bit_manipulation/readme.md), [Lock-Free Data Structures](c_c++/code/linked_lists/readme.md) |
| **5. Compiler Optimization Techniques** | Intermediate representation (IR) lowering, algorithmic strength reduction, constant multiplication synthesis (Bernstein & SCM), bit twiddling hacks, and branchless coding. | **[Compiler Optimizations Master Hub](compiler_optimization_techniques/hub_compiler_optimization_concepts.md)** | [Strength Reduction](compiler_optimization_techniques/references/strength_reduction.md), [Bit Twiddling Hacks](compiler_optimization_techniques/references/bit_twiddling_hacks.md) |
| **6. Solved Engineering Problems** | Practical hands-on firmware and silicon bringup challenges with production-grade C/C++ solutions and diagnostic tools. | **[Solved Engineering Problems Hub](solved-problems/README.md)** | [Kernel Handoff Validator](solved-problems/1-kernel-handoff-validator/problem-statement.md), [RPi 4 Bringup](solved-problems/2-raspberry-pi/bringup.md), [RPi 4 SPI EEPROM Recovery](solved-problems/2-raspberry-pi/rpi4-eeprom-recovery/SKILL.md) |
| **7. Exploration Curriculum Roadmap** | 7-Day intensive exploration plan covering boot logs, DTS bindings, JTAG/earlycon, PCIe BARs, TPU Gasket drivers, and scatter-gather DMA. | **[7-Day Exploration Plan Roadmap](exploration_plan.md)** | [Curriculum Matrix](exploration_plan.md) |

---

## 3. Core Architectural Concept Cross-Reference Matrix

```
                          ┌────────────────────────────────────────────────────────┐
                          │         CORE SYSTEM INVARIANTS & CROSS-LINKS          │
                          └───────────────────────────┬────────────────────────────┘
                                                      │
             ┌────────────────────────────────────────┼────────────────────────────────────────┐
             │                                        │                                        │
             ▼                                        ▼                                        ▼
   [ ARMv8 Privilege & Exception ]          [ Physical DRAM & MMU Paging ]          [ System Bus & Acceleration ]
   • BootROM (Secure EL3)                   • Virtual Address [47:0] Slicing          • Von Neumann Bus Bottleneck
   • TF-A BL1/BL2/BL31 (EL3 Monitor)        • TTBR0_EL1 (Userspace Root)             • PCIe Point-to-Point SerDes
   • BL33 U-Boot/Barebox (Non-Secure EL2)    • TTBR1_EL1 (Kernel Root)                • GPU SIMD / HBM Bandwidth
   • Linux Kernel head.S (Non-Secure EL1)   • PGD -> PUD -> PMD -> PTE Walk          • Systolic Array Matrix Grid
   └─► Deep Dive: Exception Levels          └─► Deep Dive: Page Tables               └─► Deep Dive: Core Computing
```

---

## 4. Operational Guidelines & Contribution Principles

1. **Master Hub Anchor Pattern:** Every major knowledge domain must maintain a top-level `HUB_*.md` or `README.md` master document serving as the architectural map, execution matrix, and relative link anchor for all underlying sub-modules.
2. **Relative Path Integrity:** All cross-document markdown links must use clean, validated relative file paths (e.g., `Link Format: [Title]` pointing to `path/to/file.md`).
3. **Zero Redundancy / Single Canonical Source:** Core technical concepts (such as Memory Barriers, Page Table Slicing, or Pointer Clockwise Parsing) have a single canonical deep-dive document. Sibling tutorials reference the canonical source rather than duplicating content.
