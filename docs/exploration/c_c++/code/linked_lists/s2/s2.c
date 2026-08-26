/*
### **2. Stacks: Zero-Allocation Interrupt Context Frame Stack**

**Concept:** Fixed Storage Stack & Memory Alignment

**Scenario:** In bare-metal or RTOS firmware, handling nested hardware interrupts requires saving interrupt frame states onto a pre-allocated stack without using dynamic allocation (`malloc`).

**Problem:** Design a thread-safe, static interrupt frame stack in C or C++.

* Define a fixed-capacity byte array that acts as the hardware stack buffer.
* Implement `bool push_context_frame(const uint32_t *regs, size_t reg_count, uint32_t align_bytes)` and `bool pop_context_frame(uint32_t *regs, size_t reg_count)`.
* **Rules:**
* Enforce strict memory alignment bounds (e.g., AAPCS requires 8-byte stack alignment at public interfaces).
* Check for stack overflow and underflow strictly without executing unsafe pointer arithmetic.
* Must be reentrant or safe against concurrent calls from different privilege levels (e.g., using atomic operations for the stack pointer).
*/

#define ARRAY_SIZE 10000
static uint8_t g_stack_array[ARRAY_SIZE] __attribute__((aligned(8)));
static size_t g_stack_top=0; // This represent the last un-used byte in the g_stack_array

static inline size_t align_up(size_t number,size_t alignment){
    if(alignment ==0) return number;
    return (number + (alignment - 1)) & ~(alignment-1);   
}

bool push_context_frame(const uint32_t *regs, size_t reg_count, uint32_t align_bytes){
    if(!regs || (reg_count==0)) return false;
    
    size_t align_start =  align_up(reg_count, align_bytes);
    size_t frame_size = reg_count * sizeof(uint32_t); 
    size_t new_stack_top = align_start + frame_size;

    if(new_stack_top > ARRAY_SIZE) return false;
    // assume adding align_byes would align the memory
    memcpy(&g_stack_array[align_start],regs, frame_size);
    
    g_stack_top = new_stack_top;
    return true;
}

bool pop_context_frame(uint32_t *regs, size_t reg_count){
    // assume push_context_frame always ensures alignment
    if(reg_count==0) return false;

    if(len(g_stack_top)<reg_count) return false;
    size_t frame_start = g_stack_top - reg_count;

    memcpy(regs, &g_stack_array[frame_start],reg_count);

    g_stack_top = frame_start;
    return true;
}