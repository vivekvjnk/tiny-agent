Sean Eron Anderson's collection (*Bit Twiddling Hacks*, hosted at Stanford) is legendary in low-level engineering. Beyond minimum/maximum calculations, it details dozens of clever branchless tricks for low-level integer manipulation.

# `max(a,b) = a ^ (a^b & -(a<b))`

This bitwise branchless trick works by using the boolean result `(a < b)` as a bitmask to dynamically select between `0` and `a ^ b`.

The core mechanics rely on two key properties of bitwise operations:

1. **Two's Complement Mask Generation:** `-(a < b)` turns a boolean comparison into an all-`1`s or all-`0`s bitmask.
2. **XOR Identity Properties:** `a ^ 0 = a` and `a ^ (a ^ b) = b`.

---

### Step 1: The Boolean-to-Mask Conversion (`-(a < b)`)

In C and C++, a comparison evaluation returns `1` for `true` and `0` for `false`. Applying the unary negation operator (`-`) converts this value using two's complement representation:

* **If $a < b$ is `false` (`0`):**
`-0` evaluates to `0` (binary `0000...0000`).
* **If $a < b$ is `true` (`1`):**
`-1` in two's complement representation turns all bits on (binary `1111...1111`, or `~0`).

| Condition | `(a < b)` | `-(a < b)` (Hex) | `-(a < b)` (Binary) |
| --- | --- | --- | --- |
| **$a \ge b$** | `0` | `0x00000000` | `00000000...00000000` |
| **$a < b$** | `1` | `0xFFFFFFFF` | `11111111...11111111` |

---

### Step 2: The Bitwise AND Masking (`(a^b) & mask`)

Next, the XOR difference between the two numbers `(a ^ b)` is masked using the bitmask generated in Step 1:

* **Case 1: $a \ge b$ (Mask is `0x00000000`)**
`(a ^ b) & 0x00000000` $\rightarrow$ `0`
* **Case 2: $a < b$ (Mask is `0xFFFFFFFF`)**
`(a ^ b) & 0xFFFFFFFF` $\rightarrow$ `a ^ b`

---

### Step 3: The Final XOR Reconstruction

Substituting the result from Step 2 back into the main expression `a ^ (...)`:

* **Case 1: When $a \ge b$ (Since $a$ is the maximum)**
`max(a, b) = a ^ 0` $\rightarrow$ **`a`**
* **Case 2: When $a < b$ (Since $b$ is the maximum)**
`max(a, b) = a ^ (a ^ b)` $\rightarrow$ **`b`**

---

### Why Use This Pattern?

This technique is a **branchless algorithm** (commonly documented in Sean Eron Anderson's *Bit Twiddling Hacks*).

By eliminating the conditionally branched `if (a < b)` statement, the processor avoids potential **branch mispredictions** inside tight loops, allowing instructions to flow smoothly through CPU pipelines without flushing.

===


# **1. Finding the Sign of an Integer (Branchless `sign()`)**
Determines whether an integer is positive (+1), negative (-1), or zero (0) without conditional branches.

```c
int sign = (v > 0) - (v < 0);

```

* **How it works:** In C, boolean expressions evaluate to `1` or `0`. If $v > 0$, `1 - 0 = 1`. If $v < 0$, `0 - 1 = -1`. If $v == 0$, `0 - 0 = 0`.

# **2. Absolute Value Without Branching**
Calculates $\vert{}v\vert{}$ using sign extension and two's complement arithmetic instead of `if (v < 0) v = -v`.

```c
int const mask = v >> (sizeof(int) * CHAR_BIT - 1); // Generates 0x00000000 or 0xFFFFFFFF
unsigned int abs_v = (v + mask) ^ mask;

```

* **How it works:** If $v \ge 0$, `mask` is `0`, yielding $(v + 0) \oplus 0 = v$. If $v < 0$, `mask` is `-1` (all 1s). Adding `-1` and XORing with `-1` flips the bits and adds 1, performing a branchless two's complement negation.

# **3. Power of Two Verification**
Checks if an integer $v > 0$ is an exact power of two (has exactly one bit set).

```c
bool is_power_of_two = v && !(v & (v - 1));

```

* **How it works:** Subtracting 1 from a power of two flips all bits from the lowest set bit downward (e.g., $8_{10} = 1000_2$, $7_{10} = 0111_2$). Bitwise ANDing $1000_2 \ \& \ 0111_2$ yields `0`.

# **4. Brian Kernighan’s Bit Counting Algorithm**
Counts the number of set bits (population count / Hamming weight) by clearing the lowest set bit in each iteration.

```c
unsigned int count = 0;
for (; v; count++) {
    v &= v - 1; // Clears the least significant set bit
}

```

* **How it works:** Instead of looping $N$ times for an $N$-bit integer, it loops only as many times as there are `1`s in the binary representation.

**5. Swapping Values Without a Temporary Variable (XOR Swap)**
Swaps two integer variables using three XOR operations.

```c
a ^= b;
b ^= a;
a ^= b;

```

* **How it works:** Uses the self-inverse property of XOR ($x \oplus x = 0$).
* Step 1: $a' = a \oplus b$
* Step 2: $b' = b \oplus a' = b \oplus (a \oplus b) = a$
* Step 3: $a'' = a' \oplus b' = (a \oplus b) \oplus a = b$



# **6. Conditionally Flipping or Negating Values**
Conditionally negates a number based on a boolean condition $f$ without using `if/else`.

```c
// Negate 'v' if 'flag' is true (1), leave 'v' unchanged if 'flag' is false (0)
int mask = -flag; // 0x00000000 or 0xFFFFFFFF
v = (v ^ mask) - mask;

```

* **How it works:** If `flag` is 0, mask is 0: $(v \oplus 0) - 0 = v$. If `flag` is 1, mask is -1: $(v \oplus -1) - (-1) = \sim v + 1 = -v$.

---

**Summary of Hacks Category Reference Table**

| Technique | Mathematical / Bitwise Basis | Key Use Case |
| --- | --- | --- |
| **Branchless `sign**` | Boolean evaluation subtraction | High-performance graphics & sorting |
| **Branchless `abs**` | Two's complement bit inversion mask | Eliminating pipeline flushes in math loops |
| **Power of Two Test** | Clear lowest set bit via $(v - 1)$ | Memory allocation & alignment checks |
| **Kernighan's Popcount** | Sparse bit set clearance | Cryptography & sparse matrix indexing |
| **XOR Swap** | Algebraic cancellation ($x \oplus x = 0$) | Zero-register register swapping |




# Miscellaneous 

Reversing all 32 bits (mirroring the bit order rather than byte order) can be done using **divide-and-conquer bit manipulation**, **lookup tables**, or **hardware instructions**.

### Method 1: Divide-and-Conquer (Parallel Bit Swap)

Instead of looping 32 times, this algorithm swaps adjacent bit groups in parallel using specific bitwise masks. It recursively swaps adjacent 1-bit pairs, 2-bit pairs, 4-bit nibbles, 8-bit bytes, and 16-bit halves.

```c
uint32_t reverse_bits_32(uint32_t x) {
    // Swap adjacent bits
    x = ((x >> 1) & 0x55555555) | ((x & 0x55555555) << 1);
    // Swap adjacent 2-bit pairs
    x = ((x >> 2) & 0x33333333) | ((x & 0x33333333) << 2);
    // Swap adjacent nibbles (4-bit groups)
    x = ((x >> 4) & 0x0F0F0F0F) | ((x & 0x0F0F0F0F) << 4);
    // Swap adjacent bytes
    x = ((x >> 8) & 0x00FF00FF) | ((x & 0x00FF00FF) << 8);
    // Swap 16-bit halves
    x = (x >> 16) | (x << 16);
    
    return x;
}

```

#### How the Masks Work

* **`0x55555555` (`01010101...`):** Isolates odd bits so they can shift right, while original even bits mask and shift left.
* **`0x33333333` (`00110011...`):** Isolates 2-bit blocks.
* **`0x0F0F0F0F` (`00001111...`):** Isolates 4-bit nibbles.
* **`0x00FF00FF`:** Isolates 8-bit bytes.

This executes in **$O(\log N)$** time (5 operations for 32 bits) without loops or branches.

---

### Method 2: Hardware Instruction (Best Performance)

Many modern CPU architectures include hardware-level bit-reversal instructions:

* **ARM64:** Has a native single-cycle bit reverse instruction: `RBIT W0, W0`.
* **GCC / Clang Built-ins:** Compilers provide compiler built-ins that emit target-specific instructions where available or fallback to optimal lowered code:
```c
// Available in recent Clang/GCC releases or as a targeted intrinsic
uint32_t rev = __builtin_bitreverse32(x);

```



---

### Method 3: 8-Bit Lookup Table (Optimized for Memory/Throughput Trade-offs)

If runtime speed is paramount on hardware lacking `RBIT` (and memory allows), precomputing a 256-entry lookup table (`BitReverseTable256`) allows reversing 32 bits using 4 table lookups combined with byte shifts:

```c
uint32_t reverse_bits_lut(uint32_t x) {
    return (BitReverseTable256[x & 0xFF] << 24) |
           (BitReverseTable256[(x >> 8) & 0xFF] << 16) |
           (BitReverseTable256[(x >> 16) & 0xFF] << 8) |
           (BitReverseTable256[(x >> 24) & 0xFF]);
}

```

---

### Comparison Matrix

| Approach | Instruction Count / Latency | Memory Footprint | Key Advantage |
| --- | --- | --- | --- |
| **Divide-and-Conquer** | ~15 operations (~5 cycles) | 0 bytes | Fully portable C, completely branchless |
| **Hardware Intrinsic (`RBIT`)** | **1 instruction (1 cycle)** | 0 bytes | Maximum throughput on supported targets |
| **Lookup Table (LUT)** | 4 indexing ops + shifts | 256 bytes | Fast on simple RISC/embedded microcontrollers |