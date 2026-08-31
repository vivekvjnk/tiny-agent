## Deterministic 3-Phase Sampling & FFT Processing Pipeline
## 1. Executive Summary
This document specifies a hard real-time data acquisition and processing architecture for the Texas Instruments AM335x SoC running the RTXC Quadros RTOS. The system requires continuous, microsecond-precision measurement of 3-phase currents and voltages via an AM1301 ADC, followed by real-time Fast Fourier Transform (FFT) analysis.
To eliminate bus arbitration delays and sampling jitter inherent in native peripherals, this design utilizes a dedicated Programmable Real-Time Unit (PRU0) for deterministic SPI bit-banging, an EDMA3 engine for zero-CPU bulk data migration, and the ARM Cortex-A8 NEON SIMD engine for accelerated mathematical computation.
------------------------------
## 2. System Architecture
The following diagram illustrates the hardware-driven, pipelined data path from physical pins to the RTOS processing task.
```text
  [ AM1301 ADC ]
        │ (3-Phase Signals)
        ▼
┌───────────────────────┐
│ PRU0 (Bit-bang SPI)   │ <── Microsecond Timed Loop
└───────────────────────┘
        │
        ▼ (Writes via local interconnect)
┌───────────────────────┐
│ PRU Shared RAM (8KB)  │ ◄── [ Ping-Pong Buffers ]
└───────────────────────┘
        │
 (EDMA Event) ◄─────── Triggered by PRU0 via INTC
        │
┌───────────────────────┐
│        EDMA3          │ ──── (Burst copy) ────┐
└───────────────────────┘                       │
                                                ▼
                                    ┌───────────────────────┐
                                    │   L3 On-Chip RAM      │ ◄── Direct Physical Buffers
                                    └───────────────────────┘
                                                │
                                       (Hardware ISR)
                                                │
                                                ▼
                                    ┌───────────────────────┐
                                    │ RTXC Processing Task  │ ◄── Wakes via Semaphore
                                    └───────────────────────┘
                                                │
                                                ▼ [ NEON Engine FFT ]
```
------------------------------
## 3. Subsystem Detailed Design## 3.1. Front-End Data Acquisition (PRU0)
The native McSPI peripheral is bypassed to eliminate inter-word state machine delays and L3 bus-mastering latency. Instead, PRU0 controls data acquisition through direct GPIO bit-banging.

* Timing Execution: PRU0 runs an un-pipelined loop at 200 MHz. Every instruction takes exactly 5 ns. Microsecond delays are implemented via exact instruction counting, achieving zero-jitter sampling.
* Phase Concurrency: PRU0 samples multiple General Purpose Input (GPI) lines simultaneously on the exact same clock edge. This eliminates the phase skew caused by sequential channel reads in standard peripherals.
* Memory Management: Raw 16-bit samples are formatted into a contiguous structured array and written into the 8KB PRU Shared Data RAM.

## 3.2. Data Transfer Mechanism (EDMA3 & Ping-Pong Buffering)
To maximize throughput and prevent data overwrites, the 8KB Shared Data RAM is partitioned into two identical spaces: Buffer Ping and Buffer Pong.

* The Handshake: When PRU0 fills Buffer Ping, it writes a block-complete status to its internal interrupt register (R31). This instantly asserts a hardware channel event mapped to the EDMA3 Controller via the PRU Interrupt Controller (INTC).
* Zero-CPU Migration: The EDMA3 engine automatically performs a high-speed burst transfer of the filled buffer into the main system memory while PRU0 concurrently fills Buffer Pong.
* EDMA PaRAM Layout: Two Parameter RAM (PaRAM) blocks are linked in a circular configuration.
* PaRAM 1 (Ping): Source = PRU Shared RAM (Ping Block); Destination = System RAM Buffer A.
   * PaRAM 2 (Pong): Source = PRU Shared RAM (Pong Block); Destination = System RAM Buffer B.

## 3.3. Memory Target & Cache Management
Because RTXC Quadros operates within a unified physical address space, complex virtual memory driver layers are avoided.

* Target Allocation: Buffers A and B are statically allocated in the AM335x internal 64KB L3 On-Chip Memory (OCMC RAM). This eliminates DDR bus arbitration overhead and maximizes read speeds for the ARM processor.
* Coherency Enforcement: Since the EDMA engine writes directly to OCMC RAM bypassing the Cortex-A8 cache, the target memory region is configured as Strongly Ordered via the MMU/MPU configuration tools to prevent the CPU from reading stale data lines.

## 3.4. Signal Processing & Execution (RTXC & NEON)

* Task Synchronization: Upon completing a block transfer, the EDMA3 hardware generates a CPU interrupt. The registered Hardware ISR clears the EDMA flag and immediately calls KS_Signal() on an RTXC Semaphore.
* Context Preservation: The RTXC task dedicated to the FFT is configured with the Floating Point/NEON context-saving flag active. This ensures NEON registers are backed up securely during unexpected RTOS task switches.
* Vectorization: The FFT task consumes data directly from the 16-byte-aligned arrays in OCMC RAM. Compiling with -O3 -mfpu=neon -mfloat-abi=hard forces the compiler to use auto-vectorization (VLD / VMUL SIMD instructions), enabling parallel calculations on four 32-bit floating-point numbers in a single cycle.

------------------------------
## 4. Key Performance Advantages

| Parameter | Standard Architecture (Linux + McSPI) | Optimized Architecture (RTXC + PRU + EDMA + NEON) |
|---|---|---|
| Sampling Jitter | High (Microseconds, bus dependent) | None (Deterministic 5 ns instruction bounds) |
| Phase Skew | Present (Sequential channel reads) | Zero (Simultaneous parallel pin sampling) |
| CPU Utilization | High (Interrupting per byte/word) | < 1% (CPU sleeps until bulk frame is ready) |
| Memory Latency | Variable (DDR Bus Contention) | Minimal (Dedicated Internal OCMC RAM) |

