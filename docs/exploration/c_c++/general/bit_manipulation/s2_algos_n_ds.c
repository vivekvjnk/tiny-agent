/*
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
