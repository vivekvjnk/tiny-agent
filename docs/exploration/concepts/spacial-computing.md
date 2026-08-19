# Non-Von Neumann Computing — Systolic Arrays & TPU Architecture

While PCIe solves bandwidth and signaling challenges at the system interconnect level, the **Von Neumann Bottleneck** remains severe inside the processor core when executing dense matrix operations ($C = A \times B$).

---

## 1. The Von Neumann Matrix Multiplication Crisis

In a classical Von Neumann processor or GPU SIMD vector lane, calculating $C_{i,j} = \sum A_{i,k} \cdot B_{k,j}$ requires:

$$\text{Memory Accesses per MAC Operation: } 2 \text{ Reads (A, B)} + 1 \text{ Write (C accumulated)} = 3 \text{ Memory Transfers}$$

For an $N \times N$ matrix multiply requiring $O(N^3)$ Multiply-Accumulate (MAC) operations, a standard CPU must perform $O(N^3)$ memory accesses over the register file/cache bus. The ALUs spend >80% of their clock cycles stalled, waiting for operands.

```
Classical Von Neumann Execution:
Memory (DRAM/Cache) <---> [ Register File ] <---> [ ALU ]
(Every single MAC requires fetching operands from memory and writing back)

```

---

## 2. The Systolic Array Principle: Data Reuse Without Memory Access

A **Systolic Array** (named after the rhythmic pumping of the human heart) breaks the Von Neumann model by piping data **directly between adjacent Processing Elements (PEs)** in a 2D grid before returning it to memory.

```
          A0,1  A0,0  ---> [ PE 0,0 ] ---> [ PE 0,1 ] --->
                             |              |
                             v              v
          B1,0  B0,0  ---> [ PE 1,0 ] ---> [ PE 1,1 ] --->

```

* **Data Rhythm:** At each clock tick, data flows horizontally (Activations) and vertically (Weights / Partial Sums).
* **Data Reuse:** An activation byte $A_{i,k}$ is read **once** from memory, then passed sequentially across 256 PEs, performing 256 calculations without a single extra memory fetch.

$$\text{Efficiency Shift: } O(N^2) \text{ Memory Bandwidth } \longrightarrow O(N^3) \text{ Compute Operations}$$

---

## 3. Google TPU (Tensor Processing Unit) Microarchitecture

Google’s TPU centers around a massive **Matrix Multiply Unit (MXU)**—a $256 \times 256$ Systolic Array containing **65,536 8-bit MAC units**.

```
+-------------------------------------------------------------------------+
|                               TPU CORE                                  |
|                                                                         |
|  +--------------------+                     +------------------------+  |
|  | Off-Chip Memory    |                     | Instruction Sequencer  |  |
|  | (DDR3 / HBM)       |                     | (CISC-like instructions)|  |
|  +---------+----------+                     +-----------+------------+  |
|            |                                            |               |
|            v                                            v               |
|  +--------------------+                     +------------------------+  |
|  |   Unified Buffer   |                     | Weight FIFO / Storage  |  |
|  |  (24 MB On-Chip    |<---+                +-----------+------------+  |
|  |     SRAM)          |    |                            |               |
|  +---------+----------+    | (Activations)              | (Weights)     |
|            |               |                            v               |
|            +---------------+---------------> +----------------------+   |
|                            |                 |  256 x 256 SYSTOLIC  |   |
|                            |                 |     ARRAY (MXU)      |   |
|                            |                 |   (65,536 MAC PEs)   |   |
|                            |                 +----------+-----------+   |
|                            |                            |               |
|                            |                            v               |
|                            |                 +----------------------+   |
|                            +-----------------| Vector Accumulators  |   |
|                                              |  & Activation Unit   |   |
|                                              +----------------------+   |
+-------------------------------------------------------------------------+

```

---

## 4. Weight-Stationary Execution Model (TPU v1–v3)

1. **Pre-loading Weights:** Weights ($B$) are streamed into the array from the top and **locked in place** inside local PE registers.
2. **Streaming Activations:** Activation vectors ($A$) stream in from the left, one column per clock cycle (staggered diagonally to match propagation delay).
3. **Internal Accumulation:** As activations flow rightward, each PE multiplies its local stationary weight with the incoming activation, adds the result to the partial sum flowing down from above, and passes the updated sum down.
4. **Output Pipeline:** Fully accumulated outputs emerge from the bottom of the array directly into the **Vector Processing Unit (VPU)** for activation functions (e.g., ReLU, Sigmoid).

---

## 5. Von Neumann vs. Systolic Architecture Comparison

| Metric | Von Neumann (CPU / GPU Core) | Systolic Array (TPU MXU) |
| --- | --- | --- |
| **Control Logic** | Complex (Out-of-order execution, branch prediction, speculative decoding) | Minimal (Simple shift register orchestration) |
| **Data Path** | Centralized Register File $\leftrightarrow$ Memory | Decentralized PE-to-PE Nearest-Neighbor Mesh |
| **Memory Access Rate** | High per operation ($O(1)$ reads per MAC) | Extensively amortized ($O(1/N)$ reads per MAC) |
| **Silicon Area Allocation** | ~70% Caches/Control, ~30% ALUs | **>80% Raw MAC Logic** |
| **Instruction Style** | Scalar/Vector (`ADD`, `MUL`, `LOAD`) | Matrix-Level (`MatrixMultiplyDecode`, `ReadWeights`) |