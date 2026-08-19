# C `const` Type Qualifier: Technical Architecture Reference

[← Back to Pointers Hub](hub_concepts.md) | [← Back to C/C++ Core Hub](../hub_concepts.md)

---

## 1. Type Semantics: C vs. C++ & Constant Expressions

In C, `const` specifies **read-only access**, not compile-time immutability.

```
                                  [ Compile-Time Integer Constant Expression (ICE) ]
                                                    /            \
                                          (C Language)          (C++ Language)
                                         /            \                |
                       [ #define N 64 ]   [ enum { N = 64 } ]    [ const int N = 64 ]
                                                                        |
                                                         (In C, this is a Read-Only Lvalue)

```

| Trait | C (`const int N = 64;`) | C++ (`const int N = 64;`) |
| --- | --- | --- |
| **Type Classification** | Read-only **lvalue** (named memory slot). | True **ICE** (Integer Constant Expression). |
| **Memory Allocation** | Allocates storage (unless fully optimized out). | Substituted inline via **constant folding**; no allocation unless address is taken. |
| **`switch(x) case N:`** | **Illegal** (Case label requires true ICE). | **Legal**. |
| **Bit-field Width** | **Illegal** (`struct { int x: N; };`). | **Legal**. |
| **Linkage** | `extern` by default (global scope). | `static` (internal linkage) by default. |

> **Constant Folding:** A compiler optimization that simplifies constant expressions at compile time (e.g., replacing `3 * 10` with `30`). In C++, `const` variables participate in constant folding. In C, only literals, macros, and `enum` constants participate.

---

## 2. Array Mechanics: Fixed-Size vs. VLA

In C99+, array dimension declarations depend on whether the size operand is a true **ICE** or an **lvalue**.

```text
Compilation Phase:
  int a[64];           ---> Known at compile time ---> Fixed-size array (Fixed stack frame layout)
  const int N = 64;
  int b[N];            ---> Evaluated at runtime  ---> Variable Length Array (VLA) (Dynamic SP adjustment)

```

### Comparative Breakdown

| Attribute | Compile-Time Fixed Array (`int a[64]`) | Variable Length Array (`int b[N]`) |
| --- | --- | --- |
| **Size Evaluated** | Translation time (Compiler). | Execution time (Runtime). |
| **Allocation Method** | Fixed stack offset calculation (`sp - offset`). | Dynamic stack pointer adjustment (`sp = sp - N*sizeof(T)`). |
| **`sizeof` Behavior** | Evaluated at compile time (ICE). | Evaluated at runtime (Emits execution code). |
| **Safety / Overhead** | Zero runtime allocation cost; stack size checked by linker. | Risk of **Stack Overflow**; cannot be caught at compile time. |
| **Standards Support** | All C standards (C89, C99, C11, C17, C23). | Mandatory in C99; **Optional** in C11+ (`__STDC_NO_VLA__`). |

---

## 3. Core Applications of `const`

```text
                          +---------------------------------------+
                          |   cmain / Startup Vector              |
                          +---------------------------------------+
                                     /                 \
                                    /                   \
            [ Flash / ROM (.rodata) ]                   [ Volatile SRAM (.data / .bss) ]
            +-----------------------+                   +-------------------------------+
            |  const LUT[256]       |                   |  uint32_t active_state        |
            |  String Literals      |                   |  int mutable_buffer[64]       |
            +-----------------------+                   +-------------------------------+

```

### 1. Embedded Memory Placement (`.rodata` vs. SRAM)

* **Global/Static Variables:** Marking data `const` routes allocation to the **`.rodata`** section (Flash/ROM) instead of `.data` (RAM).
* **Impact:** Prevents copying lookup tables from Flash to SRAM during C-runtime startup (`cmain`), saving RAM.

### 2. Parameter Defensiveness & Interface Contracts

* **Pointer Contracts:** Guarantees pass-by-reference memory immutability.
```c
void process_stream(const uint8_t *buffer, size_t len); // 'buffer' data cannot be mutated

```


* **Scalar Immutability:** Prevents accidental internal assignment within function scope.
```c
void set_duty_cycle(const uint8_t duty) {
    // duty = 100; // Compiler Error
}

```



### 3. Read-Only Function Returns

Prevents callers from modifying internal module state, static buffers, or string literals returned by reference.

```c
const char *get_firmware_version(void) {
    return "v2.1.0"; // Returns pointer to .rodata; prevents callers from overwriting chars
}

```

### 4. Immutable Struct Fields

Locks specific fields post-initialization. Enables runtime configuration parameters while preventing downstream modification.

```c
typedef struct {
    const uint32_t serial_number; // Read-only after instantiation
    uint16_t status_flags;        // Mutable
} Device;

Device dev = { .serial_number = 0xABCD1234, .status_flags = 0 };
// dev.serial_number = 0; // Compiler Error

```

### 5. Qualifier Binding Matrix (Pointer + `const`)

The position of `const` relative to `*` alters pointer mutability vs. target data mutability.

```text
      const int * p          ==>  p points to a read-only int
      int * const p          ==>  p is a read-only pointer to an int
      const int * const p    ==>  p is a read-only pointer to a read-only int

```

| Declaration Syntax | Pointer Address (`p`) | Referenced Value (`*p`) |
| --- | --- | --- |
| `int *p` | Mutable | Mutable |
| `const int *p` / `int const *p` | Mutable | **Read-Only** |
| `int * const p` | **Read-Only** | Mutable |
| `const int * const p` | **Read-Only** | **Read-Only** |

---

## 4. Hardware & Assembly Execution Semantics

```text
[ Accessing RAM (Mutable Variable) ]      [ Accessing Scalar const (Immediate Value) ]
LDR R0, =0x20000004  ; Load RAM Address   MOV R0, #64         ; Embedded directly in
LDR R0, [R0]         ; Load Value                             ; instruction opcode
(Requires Memory Bus Read Cycle)          (0 Extra Bus Cycles / Zero RAM Footprint)

```

### Performance Characteristics

1. **Scalar Optimizations:**
* Compiler converts scalar `const` values into **immediate operands** inside assembly opcodes (`MOV R0, #64`).
* Eliminates memory fetch instructions entirely.


2. **Flash (`.rodata`) Read Latency:**
* **Wait-States:** High-frequency MCUs execution from Flash may encounter wait-states (e.g., 3-5 CPU cycles) vs. zero-wait-state SRAM.
* **Mitigation:** Hardware L1 Data Caching, Flash Prefetch Buffers, or Memory Accelerators execute `.rodata` fetches at near SRAM speed.
* **Trade-off:** Eliminates RAM exhaustion and removes runtime Flash-to-RAM copy overhead during boot.

---

## Related Topics & Further Reading
* [Pointer Declaration Parsing](pointer_declaration_parsing.md) - Understanding qualifier precedence
* [`volatile` Qualifier](volatile.md) - Combining `const volatile` for hardware status registers
* [`restrict` Qualifier](restrict.md) - Exclusive access optimizations
* [`static` Keyword](static.md) - Static duration and internal linkage
