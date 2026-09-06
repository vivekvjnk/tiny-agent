
# 7-Day Systems Engineering & Platform Exploration Plan

[← Back to Exploration Master Index](README.md)

### Master Knowledge Network Hub Navigation
* **SoC Boot & Silicon Bringup:** [Cortex-A Boot Hub](concepts/HUB_ARM-Cortex-A-Boot-Process.md)
* **Core Computing & Memory:** [Core Computing Hub](concepts/HUB_Core_Computing.md)
* **Linux Kernel Internals:** [Linux Kernel Hub](linux_kernel/hub_kernel.md)
* **Systems C/C++ & Concurrency:** [Systems C/C++ Hub](c_c++/hub_concepts.md)
* **Compiler Optimizations:** [Compiler Optimizations Hub](compiler_optimization_techniques/hub_compiler_optimization_concepts.md)
* **Solved Engineering Problems:** [Solved Problems Hub](solved-problems/README.md)

---

### 7-Day Study Sprint Matrix

| Day | Primary Focus Area | Hands-On Exercise | Key Theoretical Concept |
| --- | --- | --- | --- |
| **Day 1** | SoC Boot Sequence & Power Rails | RPi 4 UART / U-Boot log tracing | POR, PMIC rail sequencing, TF-A (BL1/BL2/BL31) |
| **Day 2** | Clock, Reset & Interrupt Controllers | RPi 4 DTS modification & GIC binding | Device Tree Source (DTS), GICv2/v3, CCF, Pinmux |
| **Day 3** | Bringup Debugging, RAS & Telemetry | GCP VM / System log profiling | JTAG/SWD, `earlycon`, AER, ECC errors, I2C/PMBus |
| **Day 4** | PCIe Subsystem & BAR Mapping | GCP VM (QEMU PCIe setup) | Config Space, BAR ioremap, MSI-X, Doorbell registers |
| **Day 5** | Google Gasket Framework & Queues | Code review: `drivers/staging/gasket` | Submission/Completion ring buffers, `ioctl`, Eventfd |
| **Day 6** | Scatter-Gather DMA & Memory Pinning | C ring buffer & DMA mapping stub | `pin_user_pages`, `dma_map_sg`, Cache coherency |
| **Day 7** | Systems C, Barriers | Write lock-free SPSC queue in C | `dmb`/`dsb`/`isb`, [volatile & memory qualifiers](c_c++/pointers/volatile.md), memory barriers |

---

### Core Focus 1: SoC Bringup & Debugging Architecture

#### 1. System Power-On & Boot Chain

* **Cold Boot Sequence:** Power-On Reset (POR) released $\rightarrow$ PMIC rail stabilization (Core, Memory, I/O) $\rightarrow$ SoC ROM Code executes from SRAM $\rightarrow$ Loads TF-A BL1/BL2 $\rightarrow$ BL31 runtime setup $\rightarrow$ U-Boot (DDR PHY initialization) $\rightarrow$ Linux Kernel.
* **DDR PHY Calibration:** If the boot hangs before U-Boot main, suspect DDR PHY training failure (read/write leveling, VREF tuning, eye diagram closure).
* **Early Kernel Debugging:** Enable `earlycon` or `earlyprintk` via kernel command line parameters (`bootargs="console=ttyS0,115200 earlycon=uart8250,mmio32,0xFE215000"`) to catch kernel panics before full tty driver initialization.

#### 2. Interrupts, Clocks & Device Tree

* **GIC Architecture:** Distinguish between SPIs (Shared Peripheral Interrupts), PPIs (Private Peripheral Interrupts), and SGIs (Software Generated Interrupts).
* **Device Tree Binding:** Understand how driver matching works via `of_match_table` compatible strings.

```c
static const struct of_device_id my_tpu_dt_ids[] = {
    { .compatible = "google,tpu-bsp-ctrl", },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, my_tpu_dt_ids);

```

#### 3. Hardware Telemetry & RAS (Reliability, Availability, Serviceability)

* **In-Band / Out-of-Band Telemetry:** Sensor readouts (thermal, voltage, current) via I2C/PMBus to a System Control Processor (SCP) or BMC using MCTP over PCIe VDMs.
* **Error Classification:**
* **Correctable Errors:** Single-bit ECC memory faults, soft PCIe link retraining. Logged via interrupt/counter without crashing.
* **Uncorrectable Errors:** Multi-bit ECC errors, PCIe AER Uncorrectable Fatal errors. Trigger system halt, kernel panic, or hardware resetting sequence.



---

### Core Focus 2: TPU BSP Driver Layer in Linux

#### 1. Google Gasket ASIC Driver Framework (`drivers/staging/gasket`)

Google uses Gasket to standardise low-level character device abstractions for custom accelerators (like Edge TPU / Apex):

* **Frame Subsystems:** Gasket manages page table mapping, character device register/unregister routines, interrupt mapping, and sysfs metric exposure.
* **Co-Driver Model:** Seperate the lightweight kernel driver (handling hardware registers, interrupts, DMA, memory pinning) from the heavy user-space driver/runtime (holding scheduling logic and tensor graphs).

#### 2. Command Queues & Doorbell Handshaking

* **Ring Buffer Structures:** Memory contains two circular lock-free buffers in host/TPU shared memory:
1. **Submission Queue (SQ):** Host writes command descriptors (opcodes, DMA addresses, buffer sizes).
2. **Completion Queue (CQ):** TPU writes completion status once execution finishes.


* **Doorbell Protocol:**
1. Host appends descriptor to SQ tail in memory.
2. Host writes updated tail index to TPU's **Doorbell Register** (mapped via PCIe BAR0/BAR1).
3. TPU fetches and processes commands, updates CQ tail, and raises an **MSI-X interrupt** to host.
4. Host ISR reads CQ, executes callbacks, and wakes user-space processes waiting on `eventfd` or `poll()`.



#### 3. DMA, Page Pinning & Cache Coherency

* **Page Pinning:** User-space tensor buffers must be pinned in physical RAM to prevent OS swapping during DMA transfers:
```c
struct page **pages;
int nr_pages = pin_user_pages_fast(user_addr, count, FOLL_WRITE, pages);

```


* **Scatter-Gather Mapping:** Construct `struct scatterlist` for non-contiguous physical pages and map them for PCIe device access:
```c
int count = dma_map_sg(dev, sg_list, nents, DMA_BIDIRECTIONAL);

```


* **Cache Coherency Rules:**
* **CPU writes, Device reads:** Call `dma_sync_single_for_device()` to flush CPU cache lines to RAM before triggering the doorbell.
* **Device writes, CPU reads:** Call `dma_sync_single_for_cpu()` to invalidate CPU cache lines before reading CQ buffers.



---

### High-Frequency Interview Questions & Essential Answers

**Q1: How do you debug a system that hangs completely silently at the bootloader stage?**

* *Answer:* Check PMIC power rail power-good (`PG`) signals with an oscilloscope. Verify oscillator clock stability and reset release. Connect a JTAG debugger (e.g., TRACE32/OpenOCD) to halt the core, read CPU PC (Program Counter), inspect the stack pointer, and verify if execution is stuck in a hardware exception vector or an infinite loop inside SRAM/ROM code.

**Q2: What is the purpose of memory barriers in Linux drivers for custom accelerators?**

* *Answer:* Modern ARM64 cores and PCIe buses reorder memory operations. A CPU write to a ring buffer descriptor in DRAM must complete *before* writing to the PCIe doorbell register.
* `wmb()` / `dsb(st)` ensures all previous memory writes complete before execution proceeds.
* `writel_relaxed()` vs `writel()`: `writel()` includes a built-in memory barrier to ensure memory stores reach hardware before the register write executes.



**Q3: How does a Linux driver manage user-space zero-copy DMA to a PCIe accelerator?**

* *Answer:* User-space passes virtual address and length via `ioctl`. The driver calls `pin_user_pages_fast()` to lock physical memory pages. It builds a scatter-gather list (`dma_map_sg()`) to obtain bus/DMA addresses for the PCIe controller. The driver writes these DMA addresses to the device SQ or DMA descriptor table, triggers the DMA start register, and waits for an MSI-X interrupt indicating completion.

**Q4: What is the difference between Top Half and Bottom Half interrupt handling in a BSP driver?**

* *Answer:* Top Half (ISR) executes with interrupts disabled; it must be ultra-fast, acknowledging hardware registers, clearing the interrupt line, and scheduling a bottom half. The Bottom Half (`threaded_irq` or `tasklet`/`workqueue`) processes command completions, updates software queue indices, and notifies user-space without blocking the core.



# 31-08-2026 
- Explore TI AM335x SOC
    - Explore ARM architecture features from the perspective of this SOC
    - AXI bus 
    - Exception levels, privilege switching, MMU, cache management
    - DMA, I2C/PMBus
- PCIe Base Address Register Mapping

- DDR training stall in BL2 
- PCIe link training failures 
- CPU lock up in default exception handler
- Hardware Software register contract : RAL(Register Abstraction Layer)
- lightweight, deterministic C tests that exercise memory interfaces, AXI bus arbitration, and hardware interrupt pipelines without an OS.

