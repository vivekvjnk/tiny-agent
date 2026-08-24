# Core Systems Study Sprint

[← Back to Exploration Plan](../exploration_plan.md)

A short, high-intensity orientation plan for consolidating C/C++, embedded systems, operating systems, and ARM platform knowledge.

## Goal

Build a coherent mental model spanning:

```text
C/C++
  ↓
Memory & Concurrency
  ↓
Operating Systems
  ↓
ARM Architecture
  ↓
SoC & Memory Hierarchy
  ↓
Boot
  ↓
Kernel
  ↓
Drivers
  ↓
Hardware & Debugging
````

The emphasis is on **retrieval, implementation, and system-level connections**, not broad topic accumulation.

---

# Day 1 — C Memory & Pointers

## [Pointers Hub](pointers/hub_concepts.md)

### Detailed Topic Guides
* [Pointer Declaration Parsing](pointers/pointer_declaration_parsing.md)
* [`const` Qualifier](pointers/const.md)
* [`volatile` Qualifier](pointers/volatile.md)
* [`restrict` Qualifier](pointers/restrict.md)
* [`static` Keyword](pointers/static.md)

### Key Pointer Concepts
* Pointer arithmetic
* Arrays vs pointers
* Pointer-to-pointer
* `void *`
* Function pointers
* Alignment
* Aliasing
* Object lifetime
* Dangling pointers

## [C Memory Model](docs/exploration/c_c++/general/memory_model_n_general_concepts.md)

```text
Stack
Heap
Text / Code
.rodata
.data
.bss
```

Understand:

* Scope
* Linkage
* Storage duration
* Lifetime
* Ownership

## Bit Manipulation

* Masks
* Set / clear / toggle
* Bit extraction
* Bit fields
* Integer promotion
* Signed vs unsigned behavior
* Endianness
* Overflow behavior

## Output

Create or refine:

```text
C/C++ concepts
+
Practical problems
+
Solutions
```

---

# Day 2 — C++ & Concurrency

## C++

### Object Model

* Constructors / destructors
* RAII
* Stack vs heap objects
* Copy semantics
* Move semantics
* Rule of 0 / 3 / 5

### Polymorphism

* Virtual functions
* vtable concept
* Pure virtual functions
* Object slicing
* Compile-time vs runtime polymorphism

### Resource Management

* `std::unique_ptr`
* `std::shared_ptr`
* `std::weak_ptr`
* Ownership semantics
* Runtime cost

## Concurrency

* Race conditions
* Critical sections
* Mutexes
* Semaphores
* Spinlocks
* Atomics
* Memory ordering
* Deadlock
* Priority inversion

## Embedded Synchronization

```text
ISR ↔ Thread
DMA ↔ CPU
Producer ↔ Consumer
Shared memory
```

Understand when `volatile` is insufficient.

---

# Day 3 — Coding, OS & Embedded Linux

## Coding

Prioritize:

```text
Arrays
Strings
Pointers
Linked Lists
Stacks
Queues
Hash Tables
Trees
Bit Manipulation
```

For every problem:

```text
Clarify
  ↓
Model
  ↓
Brute Force
  ↓
Optimize
  ↓
Implement
  ↓
Test Edge Cases
  ↓
Analyze Complexity
```

Prefer depth over volume.

## Operating Systems

* Process vs thread
* User mode vs kernel mode
* Virtual memory
* Page tables
* Context switching
* Scheduling
* System calls
* IPC
* Synchronization

## Embedded Linux

```text
Bootloader
  ↓
Kernel
  ↓
Device Tree
  ↓
Driver Probe
  ↓
Hardware Initialization
  ↓
Userspace
```

Focus on understanding the complete execution chain.

---

# Day 4 — System Integration & Recall

## Reconstruct the Boot Flow

```text
Reset
  ↓
Boot ROM
  ↓
Firmware / Boot Stages
  ↓
DRAM Initialization
  ↓
Bootloader
  ↓
Kernel
  ↓
Device Tree
  ↓
Driver Initialization
  ↓
Userspace
```

## Reconstruct the SoC

```text
CPU
 ↕
Cache
 ↕
Memory
 ↕
Interconnect
 ↕
PCIe / DMA / Peripherals
```

## Reconstruct the Interrupt Path

```text
Peripheral Event
  ↓
Interrupt Controller
  ↓
CPU Exception
  ↓
ISR
  ↓
Synchronization
  ↓
Thread / Task
```

## Rapid Recall

Practice explaining concepts without notes.

Suggested categories:

```text
C/C++
Memory
Concurrency
Operating Systems
ARM
Boot
Interrupts
DMA
Cache
MMIO
Drivers
Debugging
```

---

# Study Method

For every concept:

```text
Read
  ↓
Recall
  ↓
Explain
  ↓
Implement
  ↓
Test
  ↓
Connect
```

The objective is not to accumulate isolated notes.

The objective is to build connected knowledge:

```text
Language Semantics
        ↓
Memory
        ↓
Concurrency
        ↓
OS
        ↓
Architecture
        ↓
Platform Software
        ↓
Hardware
```

---

# Repository Growth

The study repository should progressively contain:

```text
Concept Notes
├── Architecture
├── Operating Systems
├── Embedded Systems
├── Boot
├── Memory
├── C
└── C++

Practical Exploration
├── C Problems
├── C Solutions
├── C++ Problems
└── C++ Solutions

Cross-Linked Knowledge
└── Concepts connected through references and navigation hubs
```

---

# Priority

```text
★★★★★ C/C++ Fundamentals
★★★★★ Practical Coding
★★★★★ Embedded Systems
★★★★☆ Operating Systems
★★★★☆ ARM / Platform Architecture
★★★★☆ Knowledge Integration
★★☆☆☆ New Topic Exploration
```

> **Prefer connected understanding over isolated knowledge.**
>
> **Prefer recall over rereading.**
>
> **Prefer implementation over passive familiarity.**
