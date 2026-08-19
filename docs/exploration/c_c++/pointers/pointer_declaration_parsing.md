# Pointer Declaration Parsing

[← Back to Pointers Hub](hub_concepts.md) | [← Back to C/C++ Core Hub](../hub_concepts.md)

**Algorithm: Right-Left / Clockwise-Spiral Rule**
* Precedence: Postfix [] (array), () (function) > Prefix * (pointer) > Far-left Base Type.
* Qualifiers (const/volatile/restrict): Binds to token on immediate LEFT; if far-left, binds RIGHT to Base Type.
* Process: Identifier → Right → Left → Consume `)` → Far-left base type last.

**Tokens:** `[N]` = array of $N$ | `(...)` = function returning | `*` = pointer to

`const`, `volatile`, and `restrict` alter what is read-only vs. mutable based on whether they sit to the left or right of a `*`.

### Qualifier Reference Examples
| Syntax | Parsing Translation |
| --- | --- |
| `int const *p` / `const int *p` | Pointer to a **constant `int**` (data is read-only) |
| `int * const p` | **Constant pointer** to `int` (pointer address is read-only) |
| `const int * const p` | **Constant pointer** to a **constant `int**` (both read-only) |
| `int * const (*(*fn[5])())[]` | Array of 5 pointers to functions returning pointers to arrays of **constant pointers to `int**` |

### Key Patterns

| Syntax | Parsing Step | Result |
| --- | --- | --- |
| `int *p[5]` | Right: `[5]` → Left: `*` → Far Left: `int` | Array of 5 pointers to `int` |
| `int (*p)[5]` | In `()`: Left `*` → Exit → Right: `[5]` → Far Left: `int` | Pointer to an array of 5 `int`s |
| `void (*p)(int, char)` | In `()`: Left `*` → Exit → Right: `(int, char)` → Far Left: `void` | Pointer to function `(int, char)` returning `void` |

### Complex Example

```c
char *(*(*p)[3])();
```

1. **`p`** is...
2. `*p` $\rightarrow$ a **pointer to...**
3. `(*p)[3]` $\rightarrow$ an **array of 3...**
4. `*(*p)[3]` $\rightarrow$ **pointers to...**
5. `(*(*p)[3])()` $\rightarrow$ **functions returning...**
6. `char *` $\rightarrow$ **pointers to `char**`.

> **Result:** `p` is a **pointer** to an **array of 3 pointers** to **functions** returning **pointers to `char**`.

---

## Related Topics & Type Qualifiers
* [`const` Qualifier](const.md) - Constant pointers vs. pointers to const data
* [`volatile` Qualifier](volatile.md) - Pointer to volatile hardware registers
* [`restrict` Qualifier](restrict.md) - Non-aliasing pointer optimizations
* [`static` Keyword](static.md) - Storage duration & array parameter qualifiers (`[static N]`)
