

## 1. [strength reduction](references/strength_reduction.md)
Core concept: Any multiplication operation in a binary system can be decomposed into bit shift and addition

Example: Multiply X by 5
```c
result = X << 2 + X;
// left shift by 2 is equivalent to multiplication by 4. Adding one more X yields X * 5.
```

## [bit twiddling hacks](references/bit_twiddling_hacks.md)

| Technique | Mathematical / Bitwise Basis | Key Use Case |
| --- | --- | --- |
| **Branchless `sign**` | Boolean evaluation subtraction | High-performance graphics & sorting |
| **Branchless `abs**` | Two's complement bit inversion mask | Eliminating pipeline flushes in math loops |
| **Power of Two Test** | Clear lowest set bit via $(v - 1)$ | Memory allocation & alignment checks |
| **Kernighan's Popcount** | Sparse bit set clearance | Cryptography & sparse matrix indexing |
| **XOR Swap** | Algebraic cancellation ($x \oplus x = 0$) | Zero-register register swapping |