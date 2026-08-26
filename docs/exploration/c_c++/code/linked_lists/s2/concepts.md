# Core concepts

### **1. Hardware Execution Stack vs. Software Data Structure Stack**

* **The Hardware Execution Stack (`SP` register):**
This is the RAM space allocated for local variables, function parameters, and return addresses managed automatically by the CPU's Stack Pointer (`SP`). In embedded C systems, this region resides in `.bss`/SRAM as configured by the Linker Script (e.g., `_estack`).
* **The Software Data Structure Stack (Our Interrupt Context Stack):**
This is a customized ADT (Abstract Data Type) data structure—a push/pop buffer—designed to explicitly store fixed CPU register snapshots (interrupt frames) during nested hardware events.

Defining this buffer array globally puts it in static RAM (specifically the **`.bss` section** if uninitialized, or **`.data` section** if initialized). **This is fully intentional and correct.**

---

### **2. Why `.bss` / Global Static Memory IS the Right Place**

1. **Lifetime:** An interrupt frame stack must persist for the entire lifetime of the system, surviving across multiple nested ISRs and task context switches. It must never go out of scope.
2. **Access Control & Encapsulation:** It can be shared across multiple ISRs safely if accessed via atomic function interfaces (`push_context_frame`/`pop_context_frame`).
3. **Predictability (Zero Dynamic Allocation):** In safety-critical embedded systems (e.g., ISO 26262, MISRA-C), dynamic allocation (`malloc`/`free`) is strictly forbidden due to heap fragmentation risk. Pre-allocating a fixed array in `.bss` ensures memory footprint is calculated deterministically at link time.

---

### **3. Addressing Your Scope Concern**

To avoid exposing raw global variables while allowing access from any function or ISR, wrap the array inside a **file-scoped static object** or a dedicated struct.

#### **C Implementation Pattern (Encapsulated File Scope)**

```c
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define CONTEXT_STACK_CAPACITY_BYTES  2048

// Fixed byte array in .bss (static RAM)
// Shared across all functions/ISRs in this translation unit
static uint8_t g_context_stack_buf[CONTEXT_STACK_CAPACITY_BYTES] 
    __attribute__((aligned(8))); // Enforce AAPCS 8-byte alignment

static size_t g_stack_top = 0; // Byte offset pointer

// API Functions can now access g_context_stack_buf safely:
bool push_context_frame(const uint32_t *regs, size_t reg_count, uint32_t align_bytes) {
    // Accesses g_context_stack_buf
}

bool pop_context_frame(uint32_t *regs, size_t reg_count) {
    // Accesses g_context_stack_buf
}

```

#### **C++ Implementation Pattern (Struct/Class Encapsulation)**

```cpp
#include <cstdint>
#include <cstddef>
#include <atomic>

template <size_t CapacityBytes>
class InterruptFrameStack {
private:
    // Memory buffer placed in static/RAM storage
    alignas(8) uint8_t buffer_[CapacityBytes];
    std::atomic<size_t> top_{0};

public:
    bool push_context_frame(const uint32_t* regs, size_t reg_count, uint32_t align_bytes);
    bool pop_context_frame(uint32_t* regs, size_t reg_count);
};

// Instantiated globally (allocates once in .bss)
static InterruptFrameStack<2048> g_isr_stack;

```

---

### **4. Key Challenge Requirements to Keep in Mind**

Before writing the code for Challenge 2, notice these specific constraints:

1. **Alignment Math:** Stack frame pointers must be strictly calculated (e.g., rounding addresses up to `align_bytes` boundaries).
2. **Bounds/Overflow Prevention:** You must check space requirements *before* executing writes, without doing invalid pointer arithmetic past array boundaries.
3. **Concurrency/Reentrancy:** If an ISR interrupts another ISR while pushing to this stack, using standard integer increment (`g_stack_top++`) will cause a race condition. Atomic operations (like `std::atomic` or `__atomic` primitives) are necessary.




# Alignment 
align = 0100

number = 0001  0001

align -1 = 0011

offset = number

offset + (align-1) = 0001 0100

~(align-1) = 1111 1100


(offset + (align-1)) & (~(align-1)) = 0001 0100

- Hence the algorithm converts least significant group of bit an integral multiple of the align
- Irrespective of the most significant bits, this algorithm makes the entire number an integral multiple of the "align"
    - Simple logic: If any number ends in even number, it is a multiple of 2
