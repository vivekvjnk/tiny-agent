# Objective

Explore **data parallelism at two hardware levels** and understand when
parallel execution actually improves a systems workload.

-   CPU: x86 SIMD / AVX2
-   GPU: CUDA

Problems may span multiple 1-hour sessions.

------------------------------------------------------------------------

# Problem 1 --- AVX2 Mailbox Scan

## Context

Tiny Agent's scheduler scans a mailbox of:

``` c
struct Message {
    uint32_t type;
    uint32_t flags;
    uint32_t payload_size;
    uint32_t correlation_id;
};
```

Find all messages satisfying:

``` c
(flags & required_flags) == required_flags &&
payload_size >= min_size
```

## Task

Implement and benchmark:

1.  Scalar C
2.  `-O3 -march=native`
3.  Hand-written AVX2

Then inspect the generated assembly.

## Explore

-   How many messages can AVX2 process per iteration?
-   Did the compiler already vectorize the scalar version?
-   AoS vs SoA: which is better for this workload?
-   What happens when the data is not SIMD-width aligned?
-   Is the workload compute-bound or memory-bound?

## Extension

Redesign the mailbox representation for faster scanning. Decide whether
the change is actually worthwhile given FIFO semantics and message
insertion/removal.

------------------------------------------------------------------------

# Problem 2 --- CUDA Event Aggregation

## Context

Tiny Agent produces a large event stream:

``` c
struct Event {
    uint32_t agent_id;
    uint32_t type;
    uint64_t timestamp;
    float value;
};
```

For every agent, calculate:

``` text
count, sum, min, max
```

## Task

Implement and benchmark:

1.  CPU reference
2.  CUDA kernel using global atomics
3.  CUDA version reducing atomic contention

## Explore

-   How should work be divided among CUDA threads?
-   Why do atomics become a bottleneck?
-   How does `agent_id` distribution affect performance?
-   What is coalesced memory access?
-   Can shared memory reduce contention?
-   When does GPU overhead exceed the performance benefit?

## Extension

Derive the approximate workload size where:

``` text
GPU execution < CPU execution
```

Then decide whether this workload belongs in the Tiny Agent runtime at
all.

------------------------------------------------------------------------

# Session State

At the end of each hour:

``` text
Problem:
Status:
Implemented:
Measured:
Key discovery:
Unresolved:
Next step:
```

> **Goal: understand the machine, not finish the problem.**

# Problem 1 -- Solution 

## Thoughts 

Since SIMD instructions rely heavily on the exact memory locations of the input data, we have to make sure all operands of the single SIMD is placed in contiguous memory locations. 
`(flags & required\_flags) == required\_flags && payload\_size >= min\_size\` : this operation can be split into n assembly level operations:

1. Bitwise and; SIMD parallelizable; Hence all flags from all messages should be contiguous
2. Equality check; SIMD parallelizable; flags
3. Comparator operation; SIMD parallelizable; payload size

next immediate question is, how to align flag and payload_size variables from two different datastreams into a contiguous memory map?

Approach 1: Design a structure of Arrays; Each array should be of the same size of SIMD register. 
```c
#define SIMD_REGISTER_SIZE_32_BIT 4
struct mem_aligned_block {
    uint32_t flags[SIMD_REGISTER_SIZE_32_BIT];
    uint32_t payload_size[SIMD_REGISTER_SIZE_32_BIT];
}
```

### Re-orientation
- For maximum SIMD efficiency, make the bulk workload an integral multiple of the SIMD lane count; the overall data structure does not necessarily need to have that size.
Means, flags and payload_size arrays should be sized according to maximum allowed unprocessed messages per session at any instant. Splitting the bulk workload into chunks of SIMD register sizes will not yield any advantage

```c
#define SIMD_REGISTER_SIZE_32_BIT 4
#define MAX_MESSAGE_COUNT 128
struct mem_aligned_block {
    uint32_t flags[MAX_MESSAGE_COUNT];
    uint32_t payload_size[MAX_MESSAGE_COUNT];
}
```

- Still, contiguous data-structures doesn't inherently ensure contiguous memory allocation. 
We need to design a simple function to get aligned contiguous memory allocation. May be a simple wrapper over malloc.