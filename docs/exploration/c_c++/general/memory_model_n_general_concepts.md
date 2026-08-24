# C Memory Model & System Execution Architecture

---

## 1. Core C Memory Model (Standard Semantics)

The C standard dictates how compilers handle symbols, memory lifetime, and visibility through five foundational concepts.

### Scope

Scope dictates the compile-time region of code where an identifier is visible and accessible.

* **Block Scope:** Variables declared inside a block (`{}`) or function parameter list. Visible only from point of declaration to the closing brace.
* **File Scope:** Identifiers declared outside any block or function. Visible from the declaration point to the end of the Translation Unit.
* **Function Scope:** Labels used for `goto` statements (only type with function-wide visibility).
* **Function Prototype Scope:** Identifiers declared within a function declaration parameter list (terminated at the prototype boundary).

### Linkage

Linkage defines whether multiple declarations of the same identifier in different scopes or Translation Units (TUs) refer to the exact same object or function.

* **None:** The identifier refers to a unique entity. Applies to block-scope variables, structure members, and function parameters.
* **Internal Linkage (`static` at file scope):** Identifiers are visible across the entire Translation Unit in which they are declared, but invisible to the linker across other TUs.
* **External Linkage (`extern` or default file-scope functions/variables):** Identifiers refer to the same object across all TUs linked into the final binary.

### Storage Duration & Lifetime

Storage duration defines the rules for allocating and deallocating an object's memory. Lifetime is the runtime duration during which that memory contains a valid representation of the object.

```
+-------------------+--------------------------------+----------------------------+
| Storage Duration  | Lifetime                       | Primary Segment Allocation |
+-------------------+--------------------------------+----------------------------+
| Automatic         | Scope entry to scope exit      | Stack                      |
| Static            | Program startup to termination | .data, .bss, .rodata       |
| Thread            | Thread creation to termination | Thread-Local Storage (TLS) |
| Allocated         | Explicit malloc() to free()    | Heap                       |
+-------------------+--------------------------------+----------------------------+

```

### Ownership

Ownership is an architectural pattern (not enforced by the C compiler) governing which component holds the responsibility for freeing or managing a resource's lifecycle.

* **Caller-Owned:** Caller allocates space and passes a pointer; callee writes into it.
* **Callee-Owned:** Callee allocates dynamic memory on the Heap and returns ownership (pointer) to the caller, transferring `free()` responsibility.
* **Borrowed:** A function receives a pointer to inspect or mutate, but cannot store, reallocate, or free the memory beyond the call duration.

---

## 2. Compilation Model & Binary Segments

```
Source Files (.c + .h) ──> Preprocessor ──> Translation Unit ──> Compiler & Linker ──> Binary Memory Layout

```

### The Translation Unit (TU)

A Translation Unit is created when the preprocessor expands a `.c` file by inserting header contents (`#include`), resolving macros (`#define`), and processing conditional compilations (`#ifdef`). The compiler translates each TU independently into a relocatable object file (`.o`).

### Logical Memory Segments

* **`.text` (Code Segment):** Contains executable machine instructions. Marked read-only/execute.
* **`.rodata` (Read-Only Data):** Holds string literals, constant tables, and `const`-qualified global/static variables.
* **`.data` (Initialized Data):** Stores non-zero initialized global and `static` variables. Occupies physical space in the compiled binary.
* **`.bss` (Block Started by Symbol):** Holds uninitialized or zero-initialized global and `static` variables. Occupies zero storage in the executable on disk—only its total footprint size is encoded.
* **Stack:** Manages function call frames, automatic variables, register spill-over, and return addresses. Grows dynamically toward lower addresses (downward) on most architectures.
* **Heap:** Provides dynamic memory via runtime allocators (`malloc`/`free`). Grows upward toward higher addresses.

---

## 3. Execution Environment Realities

### Level 1: Bare Metal Execution Model

On bare-metal microcontrollers, physical Flash/ROM and RAM map directly into a flat memory address space without an MMU.

```
       FLASH (ROM)                              RAM
+-----------------------+              +-----------------------+
|  Interrupt Vector     |              |  .data (runtime copy) |
|  .text (Code/XIP)     |              +-----------------------+
|  .rodata              |              |  .bss (zero initialized)
|  .data (init values) ─┼──(Boot Copy)─┼─>                     |
+-----------------------+              |  Heap                 |
                                       |  v                    |
                                       |  ^                    |
                                       |  Main Stack (MSP)     |
                                       +-----------------------+

```

* **Execute-in-Place (XIP):** `.text` and `.rodata` reside permanently in Flash. The CPU fetches instructions directly over Flash buses, using hardware prefetch buffers or caches to mitigate read wait-states.
* **CRT Startup (`Reset_Handler`):** Before `main()` executes, startup code copies initial `.data` values from Flash into RAM and zeroes out the `.bss` block in RAM using boundary symbols provided by the linker script (`_sdata`, `_edata`, `_sbss`, `_ebss`).
* **RAM Functions (`.ramfunc`):** Critical ISRs or Flash-erase routines are explicitly assigned to RAM attributes. The bootloader copies these instruction bytes from Flash to RAM alongside `.data` for zero wait-state execution.

### Level 2: RTOS Execution Model

Real-Time Operating Systems operate within a single physical address space without process isolation, but introduce structured multi-threading memory constructs.

```
+----------------------------------------------------------------+
| Shared Physical RAM Address Space                              |
|                                                                |
| [.data]  [.bss]  [Global Heap]                                 |
|                                                                |
|  Task A Control Block       Task B Control Block               |
|  +-------------------+      +-------------------+              |
|  | Task A Private    |      | Task B Private    |              |
|  | Stack (PSP)       |      | Stack (PSP)       |              |
|  +-------------------+      +-------------------+              |
+----------------------------------------------------------------+

```

* **Flat Address Space:** Tasks share all global variables, `.data`, `.bss`, and Heap segments. No hardware memory protection exists unless an MPU (Memory Protection Unit) is configured to generate access faults.
* **Thread Stacks:** Each RTOS task is allocated a discrete stack array within RAM (either dynamically via task creation or statically pre-allocated).
* **Context Switching:** The CPU switches stack pointers (e.g., from Main Stack Pointer `MSP` to Process Stack Pointer `PSP` on ARM Cortex-M) during context switches to isolate stack frames across tasks.

### Level 3: Rich OS (Linux / MMU) Execution Model

Operating systems leverage the Memory Management Unit (MMU) to map physical RAM into isolated Virtual Address Spaces for each running process.

```
Virtual Address Space (Per Process)
0x7FFFFFFFFFFF +-----------------------+
               |  Environment & Stack  | (Grows Downward)
               |  v                    |
               |                       |
               |  ^                    |
               |  Heap                 | (Grows Upward via brk/sbrk)
               +-----------------------+
               |  Memory Mapping Segment| (mmap shared libraries, anonymous pages)
               +-----------------------+
               |  .bss                 |
               |  .data                |
               |  .rodata              |
               |  .text                |
0x000000000000 +-----------------------+

```

* **On-Demand Page Loading:** ELF execution binaries are mapped into virtual memory. Physical RAM pages are allocated lazily via page faults when code execution reaches an unmapped page.
* **Kernel Boundary & Protection:** Memory is divided into User Space and Kernel Space. Hardware page table flags enforce access permissions (`Read`, `Write`, `Execute`) per page, throwing a Segmentation Fault (`SIGSEGV`) on violation.
* **System Calls:** The heap manager requests additional virtual address allocations from the kernel using `brk()`/`sbrk()` (adjusting the program break) or `mmap()` (allocating distinct virtual memory regions).