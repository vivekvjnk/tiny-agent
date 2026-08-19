# C `restrict` Type Qualifier: Technical Architecture Reference

[← Back to Pointers Hub](hub_concepts.md) | [← Back to C/C++ Core Hub](../hub_concepts.md)

---

## 1. Core Semantics & The Pointer Aliasing Problem

Introduced in C99, `restrict` is a type qualifier that applies **exclusively to pointers**. It is a binding contract between the programmer and the compiler promising that for the lifetime of the pointer, **no other pointer will be used to access the memory object it points to**.

```text
[ Non-Restricted Pointers (Alias Risk) ]         [ Restricted Pointers (Guaranteed Disjoint) ]
  ptrA ───┐                                         ptrA ───────► [ Buffer A ] (Exclusive Access)
          ├───► [ Memory Region ] (Might overlap)
  ptrB ───┘                                         ptrB ───────► [ Buffer B ] (Exclusive Access)
(Compiler must reload memory after stores)       (Compiler can cache loads in CPU registers)

```

* **Pointer Aliasing:** Occurs when two pointers refer to the same memory location.
* **Compiler Dilemma:** Without `restrict`, the compiler must conservatively assume that writing through pointer `B` might modify the value being read through pointer `A`.
* **Optimization Guarantee:** `restrict` asserts no aliasing exists, allowing the compiler to perform aggressive optimizations like load elision, instruction reordering, register caching, and SIMD vectorization.

---

## 2. Compiler Optimization Impact & Assembly Breakdown

Consider a function adding two arrays into an output array:

### 1. Without `restrict` (Conservative Code Generation)

```c
void add_arrays(int *a, int *b, int *out, int n) {
    for (int i = 0; i < n; i++) {
        *out += *a + *b; // Compiler fears 'out' might overlap with 'a' or 'b'
    }
}

```

```assembly
; Loop body without restrict (ARM Assembly)
.L3:
    LDR R3, [R0]        ; Load *a from RAM
    LDR R4, [R1]        ; Load *b from RAM
    ADD R3, R3, R4      ; R3 = *a + *b
    LDR R4, [R2]        ; MUST reload *out from RAM (in case previous loop modified it)
    ADD R3, R3, R4      ; R3 = *out + (*a + *b)
    STR R3, [R2]        ; Store result to *out in RAM
    SUBS R3, R3, #1
    BNE .L3

```

### 2. With `restrict` (Aggressive Register Caching & SIMD)

```c
void add_arrays(int * restrict a, int * restrict b, int * restrict out, int n) {
    for (int i = 0; i < n; i++) {
        *out += *a + *b;
    }
}

```

```assembly
; Loop body with restrict (ARM Assembly)
.L3:
    ; Loads of *a and *b are cached across iterations; memory re-loads eliminated.
    ; Enables loop vectorization (NEON/AVX 128-bit/256-bit SIMD registers):
    VLD1.32 {q0}, [R0]!  ; Vector Load 4 ints from 'a' at once
    VLD1.32 {q1}, [R1]!  ; Vector Load 4 ints from 'b' at once
    VADD.I32 q0, q0, q1  ; Parallel SIMD Addition
    VLD1.32 {q2}, [R2]   ; Vector Load 4 ints from 'out'
    VADD.I32 q0, q0, q2  ; Parallel SIMD Accumulation
    VST1.32 {q0}, [R2]!  ; Vector Store 4 ints to 'out'

```

---

## 3. Syntax Placement & Scope Rules

`restrict` modifies the **pointer variable itself**, not the underlying type data. Therefore, it is placed to the **right** of the asterisk `*`.

```text
      int * restrict p          ==>  p is a restricted pointer to a mutable int
      const int * restrict p    ==>  p is a restricted pointer to a read-only int
      int * restrict * p        ==>  p is a normal pointer to a restricted pointer to an int

```

| Declaration Syntax | Pointer Modifiable? | Target Modifiable? | Aliasing Permitted? |
| --- | --- | --- | --- |
| `int *p` | Yes | Yes | **Yes** (Conservative compilation) |
| `int * restrict p` | Yes | Yes | **No** (Optimizations enabled) |
| `const int * restrict p` | Yes | **No** | **No** (Optimizations enabled) |
| `int * const restrict p` | **No** | Yes | **No** (Optimizations enabled) |

---

## 4. Standard Library Case Study: `memcpy` vs. `memmove`

The C standard library explicitly leverages `restrict` in functions to define runtime memory bounds contracts:

```c
// 1. memcpy uses 'restrict' -> Buffers MUST NOT overlap
void *memcpy(void * restrict dest, const void * restrict src, size_t n);

// 2. memmove does NOT use 'restrict' -> Buffers CAN overlap safely
void *memmove(void *dest, const void *src, size_t n);

```

| Trait | `memcpy` | `memmove` |
| --- | --- | --- |
| **`restrict` Qualifiers** | **Yes** (`dest` and `src` are restricted) | **No** |
| **Performance** | Maximum speed (SIMD vector copies, loop unrolling) | Slightly slower (checks overlap, handles copy direction) |
| **Overlap Behavior** | **Undefined Behavior (UB)** if memory regions overlap | Safe handling of overlapping memory regions |

---

## 5. Undefined Behavior & Edge Cases

`restrict` is a **pure compiler assertion**. The C runtime performs no verification checks. If you break the contract, the compiler generates incorrect code without throwing a warning.

```c
void compute(int * restrict a, int * restrict b, int *val) {
    *a = 5;
    *b = 10;
    *val = *a; // Compiler replaces this with '*val = 5' via constant propagation
}

int main(void) {
    int x = 0;
    compute(&x, &x, &x); // UNDEFINED BEHAVIOR! 'a' and 'b' alias the exact same variable.
    // Result: x may unexpectedly equal 5 or 10 depending on compiler optimization flags (-O2/-O3).
}

```

### Scope Restriction Rules

A `restrict` pointer obligation is active only within its **block scope**:

```c
void process(int * restrict p, int * q) {
    // Within this scope, 'p' is guaranteed exclusive access to its memory.
    {
        int * local_p = p; // OK: Aliasing inside local scope derived from 'p'
        *local_p = 10;
    }
}

---

## Hardware Execution & SIMD Vector Pipelines
For details on how vector queues and vector register files enable SIMD acceleration during `restrict` loop optimizations, see the [CPU Core Architecture Diagram](resources/cpu_core.png).

---

## Related Topics & Further Reading
* [`const` Qualifier](const.md) - Read-only pointer contracts
* [`volatile` Qualifier](volatile.md) - Preventing load elision vs. enabling `restrict` optimizations
* [Pointer Declaration Parsing](pointer_declaration_parsing.md) - Parsing `restrict` pointer syntax
* [`static` Keyword](static.md) - C99 `[static N]` array parameter qualifiers


```