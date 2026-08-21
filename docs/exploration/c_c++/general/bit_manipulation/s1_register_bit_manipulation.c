/* 1.1 
Write a function or macro in C to set, clear, or toggle a specific field of $N$ bits starting at bit offset $K$ inside a 32-bit register `uint32_t *reg`, without modifying any surrounding bits. Ensure your solution handles $N=32$ correctly without undefined behavior from shift overflows (`<< 32`).
*/

//Single bit operations
int set_bit(uint32_t *reg,uint32_t bit_number){
    *reg = *reg | (1UL << bit_number);
}


int reset_bit(uint32_t *reg,uint32_t bit_number){
    *reg = *reg & ~(1UL << bit_number);
}


int toggle_bit(uint32_t *reg,uint32_t bit_number){
    *reg = *reg ^ (1UL << bit_number);
}

//Handling n bits 
/*Generate mask for N bits over offset K*/
int mask_gen(uint32_t N, uint32_t K){
    if(N==0) return 0;
    uint32_t mask = (N>=32) ? (1UL << N) -1 : 0xFFFFFFFFUL; //handle overflow case
    return mask<<K;
}

int set_bits(uint32_t *reg,uint32_t number_of_bits,uint32_t offset){
    if (!reg || (number_of_bits + offset > 32)) return;
    uint32_t mask = mask_ge(number_of_bits,offset);
    *reg |= mask;
}

int reset_bits(uint32_t *reg,uint32_t number_of_bits,uint32_t offset){
    if (!reg || (number_of_bits + offset > 32)) return;
    uint32_t mask = mask_ge(number_of_bits,offset);
    *reg &= ~mask;
}

int toggle_bits(uint32_t *reg,uint32_t number_of_bits,uint32_t offset){
    if (!reg || (number_of_bits + offset > 32)) return;
    uint32_t mask = mask_ge(number_of_bits,offset);
    *reg ^= mask;
}

int field_write(uint32_t *reg, uint32_t val, uint32_t N, uint32_t K){
    if (!reg || (number_of_bits + offset > 32)) return;
    uint32_t mask = mask_ge(number_of_bits,offset);
    *reg = (*reg & ~mask) | ((val & (mask>>K)) << K);
}



/* 1.2
You are designing a high-throughput lock-free ring buffer where the size $S$ is constrained to be a power of 2 ($S = 2^k$). Write a C function `uint32_t get_buffer_index(uint32_t absolute_index, uint32_t buffer_size)` that maps `absolute_index` to `[0, S-1]` using bit manipulation instead of modulo (`%`). Include a compile-time/runtime check to assert that $S$ is valid.
*/

uint32_t get_buffer_index(uint32_t absolute_index, uint32_t buffer_size){
    // typical modulo operation : absolute_index % buffer_size
    // Prepare a mask by negating 1 from S. Then and absolute_index with this mask
    return absolute_index & (buffer_size -1) ; 
}

/* 1.3
Implement an efficient $O(1)$ function in C/C++ to convert a 32-bit integer from Little-Endian to Big-Endian (or vice versa) using only bitwise shifts, masks, and OR operations. What ARM or x86 assembly instructions correspond to this, and how can you hint it to the compiler?
*/
uint32_t endian_convert(uint32_t x) {
    return ((x & 0x000000FFU) << 24) | // First byte to last byte
           ((x & 0x0000FF00U) <<  8) | // second byte to 3rd byte
           ((x & 0x00FF0000U) >>  8) | // 3rd byte to second byte
           ((x & 0xFF000000U) >> 24); // Last byte to first byte
}
/*
### Assembly Instructions & Compiler Hints

Modern compilers recognize this specific 4-shift byte-swap pattern and compile it down to a single instruction instead of executing multiple shifts and ORs.

#### 1. Hardware Assembly Instructions

* **ARM (v6+ / ARM64):** `rev r0, r0`
* **x86 / x86_64:** `bswap eax`

#### 2. Compiler Intrinsics (Explicit Hints)

To guarantee optimal single-cycle assembly execution across compilers without relying on idiom recognition:

* **GCC / Clang:** `__builtin_bswap32(x)`
* **MSVC:** `_byteswap_ulong(x)`
* **C++23 Standard Library:** `#include <bit>` $\rightarrow$ `std::byteswap(x)`

*/