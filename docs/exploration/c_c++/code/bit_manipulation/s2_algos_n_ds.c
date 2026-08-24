/* s2.1
Implement `int count_leading_zeros(uint32_t x)` without using hardware intrinsics (e.g., `__builtin_clz`). Optimize it for execution speed ($O(\log N)$ steps using binary reduction/masking).


**Counting Leading Zeros (CLZ)** means counting the number of zero bits that appear **before** the first set bit (the most significant `1` bit), starting from the most significant bit (MSB, bit 31).

If $x = 0$, all 32 bits are zero, so the answer is **32**.

### Examples for a 32-bit Integer

| Input $x$ (Hex) | Input $x$ (32-bit Binary) | Leading Zeros | First `1` Bit Position |
| --- | --- | --- | --- |
| `0x00000000` | `0000 0000 0000 0000 0000 0000 0000 0000` | **32** | None |
| `0x80000000` | `1000 0000 0000 0000 0000 0000 0000 0000` | **0** | Bit 31 |
| `0x00000001` | `0000 0000 0000 0000 0000 0000 0000 0001` | **31** | Bit 0 |
| `0x000F0000` | `0000 0000 0000 1111 0000 0000 0000 0000` | **12** | Bit 19 |

---

### $O(\log N)$ Binary Search Strategy


### Hardware Equivalent & Intrinsics
In actual firmware production code, compilers map this entire operation to dedicated CPU instructions:

* **ARM Architecture:** `CLZ r0, r0`
* **x86 Architecture:** `LZCNT eax, eax` or `BSR`
* **GCC / Clang Intrinsic:** `__builtin_clz(x)` *(Note: `__builtin_clz(0)` is undefined behavior, so always check for `x != 0` first).*
*/

int count_leading_zeros(uint32_t x){
    int zero_count = 0;
    //check upper 16bits
    if((x & 0xFFFF0000) == 0){
        zero_count +=16;
        x <<=16;
    }
    //check upper 8bits
    if((x & 0xFF000000) == 0){
        zero_count +=8;
        x <<=8;
    }
    //check upper 4bits
    if((x & 0xF0000000) == 0){
        zero_count +=4;
        x <<=4;
    }
    //check upper 2bits
    if((x & 0xC0000000) == 0){
        zero_count +=2;
        x <<=2;
    }
    //check upper 1bits
    if((x & 0x80000000) == 0){
        zero_count +=1;
    }

    return zero_count;
}


/* s2.2
Write a function `uint32_t reverse_bits(uint32_t x)` that reverses the order of all 32 bits in an unsigned integer.
* *Follow-up:* Provide an $O(1)$ divide-and-conquer mask technique (parallel bit swapping) that runs in 5-6 operations.
*/
/*
Use cases

| Domain | Primary Use Case | Key Mechanism |
| --- | --- | --- |
| **Digital Signal Processing** | In-place Radix-2 FFT algorithms | Reorders inputs/outputs into bit-reversed order for $O(N \log N)$ butterfly computation. |
| **Compiler Optimizations** | Lowering bit-reversal operations on target architectures | Fallback sequence for `__builtin_bitreverse32` on CPUs lacking native hardware instructions (e.g., x86). |
| **Computer Graphics** | Quasi-Monte Carlo rendering & sampling | Computes Van der Corput / Halton sequences to distribute sample rays uniformly across screens. |
| **Spatial Indexing** | Spatial hashing & Morton encoding (Z-order curves) | Interleaves or permutes bits to map multi-dimensional coordinates onto 1D memory indices. |
| **Telecommunications** | OFDM subcarrier mapping in 4G/5G and Wi-Fi | Scrambles and interleaves bits to prevent burst errors over wireless channels. |
| **Cryptography** | Cipher diffusion & S-box permutation | Rearranges bit vectors to optimize confusion/diffusion layers in lightweight encryption. |
*/
uint32_t flip_bits(uint32_t x){
    /*
    Logic: 
    Step 1: reverse groups of 1 bits 
        ie: 1101 0100 --> 1110 1000 : (Use mask 1010(A) --> right shift once) or (Use mask 0101(5) --> left shift once)
    Step 2: reverse groups of 2 bits
        ie: 1110 1000 --> 1011 0010 : (Use mask 1100(C) --> right shift twice) or (Use mask 0011(3) --> left shift twice)
    Step 3: reverse groups of 4 bits
        ie: 1011 0010 --> 0010 1011 : (Use mask 1111 0000(F0) --> right shift by 4) or (Use mask 0000 1111(0F) --> left shift by 4)
    ...    
    */
    x = ((0xAAAAAAAA & x) >>1) | ((0x55555555 & x) << 1);
    x = ((0xCCCCCCCC & x) >>2) | ((0x33333333 & x) << 2);
    x = ((0xF0F0F0F0 & x) >>4) | ((0x0F0F0F0F & x) << 4);
    x = ((0xFF00FF00 & x) >>8) | ((0x00FF00FF & x) << 8);
    x = ((0xFFFF0000 & x) >>16) | ((0x0000FFFF & x) << 16);
    return x;
}


/* s2.3
Write inline C/C++ functions `uint32_t rotate_left(uint32_t value, uint8_t shift)` and `uint32_t rotate_right(uint32_t value, uint8_t shift)` that correctly perform circular rotations on 32-bit words for any shift amount $0 \le \text{shift} < 256$ without invoking undefined behavior.

Use cases:
| Domain | Primary Use Case | Key Mechanism |
| --- | --- | --- |
| **Cryptography & Hashing** | Block ciphers (AES, ChaCha20, Blowfish) & hashing (SHA-2, MD5) | Spreads bit state across registers via dynamic bit permutations and fixed internal rounds (e.g., quarter-rounds in ChaCha20). |
| **Compiler Optimization** | Multi-precision arithmetic & bitwise emulation | Emulates vector primitives, optimizes bitwise shift/mask sequences, and lowers idioms to native `ROL`/`ROR` assembly instructions. |
| **Embedded & Systems** | Device drivers, register manipulation & hardware protocols | Shifts serial data bit-streams (SPI, I2C, UART) into payload byte boundaries and updates packed status registers. |
| **Cyclic Redundancy Checks** | Frame check sequence validation (Ethernet, CAN, USB) | Aligns polynomial divisions across word boundaries during continuous hardware CRC framing and error-detection steps. |
| **Pseudo-Random Numbers** | PRNG generators (xorshift, Permuted Congruential Generators) | Rotates state vectors to break linear patterns and maximize statistical randomness across high-order and low-order bit positions. |
| **Graphics & Game Engines** | Bitboard manipulation in board games & spatial packing | Rotates 64-bit/32-bit grid matrices (e.g., Chess bitboards) for directional move generation and directional masking logic. |

*/
uint32_t rotate_left(uint32_t value, uint8_t shift){
    uint8_t s = shift & 31;
    return s ? (value << s) | (value >> (32 - s)) : value;
}


uint32_t rotate_right(uint32_t value, uint8_t shift){
    uint8_t s = shift & 31; 
    return s ? (value >> s) | (value << (32 - s)) : value;
}
