### **1. Hardware & Register Manipulation**
[Solutions are available here](s1_register_bit_manipulation.c)

* **1.1. Atomic Bit Field Clearing/Setting**
Write a function or macro in C to set, clear, or toggle a specific field of $N$ bits starting at bit offset $K$ inside a 32-bit register `uint32_t *reg`, without modifying any surrounding bits. Ensure your solution handles $N=32$ correctly without undefined behavior from shift overflows (`<< 32`).
* **1.2. Ring Buffer Bitmask Indexing**
You are designing a high-throughput lock-free ring buffer where the size $S$ is constrained to be a power of 2 ($S = 2^k$). Write a C function `uint32_t get_buffer_index(uint32_t absolute_index, uint32_t buffer_size)` that maps `absolute_index` to `[0, S-1]` using bit manipulation instead of modulo (`%`). Include a compile-time/runtime check to assert that $S$ is valid.
* **1.3. Endianness Swap & Register Packing**
Implement an efficient $O(1)$ function in C/C++ to convert a 32-bit integer from Little-Endian to Big-Endian (or vice versa) using only bitwise shifts, masks, and OR operations. What ARM or x86 assembly instructions correspond to this, and how can you hint it to the compiler?

---

### **2. Systems Algorithmic & Data Structures**
[Solutions are available here](s2_algos_n_ds.c)
* **2.1. Find First Set Bit / Leading Zero Counting**
Implement `int count_leading_zeros(uint32_t x)` without using hardware intrinsics (e.g., `__builtin_clz`). Optimize it for execution speed ($O(\log N)$ steps using binary reduction/masking).
* **2.2. Bit-Reversal of a Register**
Write a function `uint32_t reverse_bits(uint32_t x)` that reverses the order of all 32 bits in an unsigned integer.
* *Follow-up:* Provide an $O(1)$ divide-and-conquer mask technique (parallel bit swapping) that runs in 5-6 operations.


* **2.3. Circular Bit Rotation (ROL / ROR)**
Write inline C/C++ functions `uint32_t rotate_left(uint32_t value, uint8_t shift)` and `uint32_t rotate_right(uint32_t value, uint8_t shift)` that correctly perform circular rotations on 32-bit words for any shift amount $0 \le \text{shift} < 256$ without invoking undefined behavior.

---

### **3. Hardware Driver & Memory Mapping Problems**

* **3.1. Bitmap Allocator for DMA Buffers**
A hardware engine manages 64 discrete DMA buffers tracked by a `uint64_t bitmap` (where `0` represents free and `1` represents occupied).
* Write `int allocate_buffer(uint64_t *bitmap)` to find and claim the lowest index free buffer atomically/efficiently.
* Write `void free_buffer(uint64_t *bitmap, int index)` to return it.


* **3.2. Parity & Error Correction (Hamming Distance / ECC)**
* **Part A:** Write a function `uint8_t compute_parity(uint32_t x)` that returns `1` if `x` has an odd number of set bits and `0` if even, using parallel XOR operations in $O(\log N)$.
* **Part B:** Given two 64-bit integer bitmasks representing sensor status registers, write an optimal function to calculate their Hamming distance (count of differing bit positions).


* **3.3. Check Power of Two and Nearest Power Alignment**
* Write a macro or static inline function `is_power_of_two(uint32_t x)` that evaluates to true/false in $O(1)$.
* Write `uint32_t align_up_power_of_two(uint32_t x)` which rounds up a 32-bit integer to the nearest power of 2 (e.g., `13` $\rightarrow$ `16`, `16` $\rightarrow$ `16`).

