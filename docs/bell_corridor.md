**WT**: Here's the 7 days [exploration plan](exploration/exploration_plan.md)
And here's the summary of topics:

| Day | Primary Focus Area | Hands-On Exercise | Key Theoretical Concept |
| --- | --- | --- | --- |
| **Day 1** | SoC Boot Sequence & Power Rails | RPi 4 UART / U-Boot log tracing | POR, PMIC rail sequencing, TF-A (BL1/BL2/BL31) |
| **Day 2** | Clock, Reset & Interrupt Controllers | RPi 4 DTS modification & GIC binding | Device Tree Source (DTS), GICv2/v3, CCF, Pinmux |
| **Day 3** | Bringup Debugging, RAS & Telemetry | GCP VM / System log profiling | JTAG/SWD, `earlycon`, AER, ECC errors, I2C/PMBus |
| **Day 4** | PCIe Subsystem & BAR Mapping | GCP VM (QEMU PCIe setup) | Config Space, BAR ioremap, MSI-X, Doorbell registers |
| **Day 5** | Google Gasket Framework & Queues | Code review: `drivers/staging/gasket` | Submission/Completion ring buffers, `ioctl`, Eventfd |
| **Day 6** | Scatter-Gather DMA & Memory Pinning | C ring buffer & DMA mapping stub | `pin_user_pages`, `dma_map_sg`, Cache coherency |
| **Day 7** | Systems C, Barriers | Write lock-free SPSC queue in C | `dmb`/`dsb`/`isb`, volatile, memory barriers |

**CO**: Ok.. so we are starting from SoC Boot sequence and power rails. 
We already have raspbian os in our pi. 

**CO**: Following are few of the key concepts from TFA boot sequence and power rails

[TFA Services](exploration/concepts/TFA.md)
===========
1. PSCI : Power State Coordination Interface
    - Unified discoverable API for 
        - CPU on/off
        - System sleep 
        - CPU Idle
    - Standard on ARMv8 systems, ARMv7 also supports
    - Solves the problems of 
        - Different power sequencing for different CPUs
        - Standardized API for high level CPU management operations like sleep, idle management etc. For low power implementations, a separate controller can maintain the minimum functionalities of the system while the main CPU is off, then trigger the main CPU from this low power controller when required. Such implementations are capable of working with all PSIC supported ARM devices through the API.
    - STM32 uses PSCI in arm32 systems 
2. SCMI : System Control and Management Interface
    - Unified discoverable API for clocks, power,...
    - Simplified control interface for linux kernel 

[Device Trees](exploration/concepts/Device-Tree.md) 
============
- The problem:
    - Many slightly different SOC versions may branch out from one CPU family
    - Embedding all of their features in Linux kernel is not a scalable approach 
    - Hence Linux kernel maintains a device tree

- Files which describe individual hardware(SOCs)
    - Read and understand capabilities of Raspberry PI Device tree
    - Shared components across different versions of SOCs would use the same kernel driver. Kernel resolves driver for specific peripherals from Device tree.

- Device trees are stored in .dtb files
    - .dts is the human readable source text file of the device tree
    - .dts: device tree source/string, .dtb: device tree binary

[PMIC and Hardware Power-on Reset(POR) State Machine](exploration/concepts/ARM-Cortex-A-Boot-Process.md)
Before BL1 even runs, hardware power rail sequencing must succeed:

* **Rail Sequencing:** PMIC regulators ramp in a strict hardware sequence: $V_{DD\_CORE} \rightarrow V_{DD\_MEM} \rightarrow V_{DD\_IO}$ to prevent CMOS latch-up.
* **Clock Lock & Reset:** Main crystal oscillator starts $\rightarrow$ System PLLs lock $\rightarrow$ PMIC detects stable power via Power-Good (`PG`) signals $\rightarrow$ Asserts and releases `POR_N` (Power-On Reset).
* **Boot Straps:** Primary core un-clears reset, reads physical HW pin straps (boot device selection: eMMC vs SPI vs PCIe endpoint mode), and jumps to the hardcoded BootROM execution address.

[BL2](exploration/concepts/BL2.md)
Primary responsibilities:
1. DDR PHY Initialization & Training (Read/Write leveling, $V_{\text{REF}}$ calibration) .
2. Secure Boot Authentication (Validates RSA/ECC signatures against ROTPK) .
3. Downstream Binary Loading (Fetches BL31, BL32, BL33 into DRAM).

**Why DDR PHY Training Happens in BL2 (SRAM)**:
* **The Problem:** DRAM (DDR4/DDR5/HBM) cannot be read or written to upon cold boot because board trace delays, voltage variations, and impedance misalignment distort bit timings.
* **The Solution:** BL2 executes strictly inside internal On-Chip SRAM (typically 128KB–512KB). It initializes the DDR PHY controller, running DQS read/write leveling, eye-diagram training, and $V_{\text{REF}}$ calibration. Once training completes, main DRAM is mapped and BL33 (U-Boot/Barebox) is fetched into DRAM.


[BL33](exploration/concepts/BL33.md): [Barebox Proper](exploration/tools/Barebox.md)
===================
Barebox is an alternative to U-Boot

- Runs in [Exception Level](exploration/concepts/Exception-Levels-in-ARM.md) 2
    - Barebox can be run under EL1. ARM supports this
- Support for many communication interfaces
    - Networking/NFS Boot 
    - Bootspec parsing
    - USB Gadget support
        - Serial 
        - Mass storage 
        - fastboot
        - NOTE: This is how we boot linux images from USB drives in an ARM PC

BL33: Kernel start 
=================
- Decompress kernel 
    - [Kernel decompression](exploration/concepts/Kernel-Decompression.md) is the process where an operating system's compressed core file unpacks itself into computer memory (RAM) during system startup
    - Aarch64 doesn't implement decompression in the kernel
- Load device tree file(DTB)
- Mask interrupts
    - Interrupt handlers are uninitialized at this stage 
    - Interrupts occurred during BL33: Kernel start process may force program counter to jump to un-initialized memory locations and crash the system

    - [Generic Interrupt Controller(GIC)](exploration/concepts/Generic-Interrupt-Controller.md) is not fully initalized 
    - Atomic state requirement: Core intialization tasks like *setting up page tables, stack pointers and CPU features* must run without interruption to maintain data integrity.
- Initialize [standard ARM timer](exploration/concepts/standard-arm-timer.md)
    - SysTick timer in Cortex M microcontroller processors
    - Generic Timer in Cortex A processors 

- Load kernel at offset specified in header 
- Disbale MMU, Data caches 
- Initialize CPU registers for either EL2 or EL1
    - e.g. x0 for device tree blob(.dtb) 
- Jump to the kernel entry point(`head.S`)

When BL33 jumps to the Linux kernel entry point (`head.S`), the hardware state **must** strictly satisfy this contract:
* **`x0` Register:** Physical memory address of the Device Tree Blob (`.dtb`).
* **`x1`, `x2`, `x3` Registers:** Reserved (must be set to `0`).
* **MMU:** Disabled. (why?)
* **Data Caches:** Disabled (prevents stale DRAM cacheline hits).
* **Instruction Cache:** Can be enabled or disabled.
* **Interrupts:** All asynchronous interrupts (PSTATE `.D`, `.A`, `.I`, `.F` flags) masked.



Miscellaneous
=============
Exception Levels for CPU registers : Document under Exception Level document
- Vendor specific 
- iMX 8 system has Central Security concept
    - It defines the EL of the memory map

**WT**: Here's the compiled boot process table with exception level mapping for ARM Cortex A series processor

|Stage |Software Component         |Execution Level (EL)|Memory Context  |Key Responsibility                                        |
|------|---------------------------|--------------------|----------------|----------------------------------------------------------|
|BL1   |Mask BootROM               |EL3 (Secure)        |Internal SRAM   |HW Boot strap evaluation, loads BL2                       |
|BL2   |Trusted Boot Firmware      |EL3 / EL1 (Secure)  |Internal SRAM   |Clock tree, PMIC init, DDR PHY training, loads BL31/BL33  |
|BL31  |TF-A Runtime Monitor       |EL3 (Secure)        |Secure SRAM/DRAM|Handles PSCI (power/CPU on/off), SMC calls, EL3 exceptions|
|BL33  |Bootloader (U-Boot/Barebox)|EL2 (Non-Secure)    |Main DRAM       |Network/USB boot, parses DTB, loads Kernel image into RAM |
|Kernel|Linux Kernel (Image)       |EL1 (Non-Secure)    |Main DRAM       |Virtual memory management, driver initialization          |

You can find detailed documentation on each stage of the boot process [here](exploration/concepts/ARM-Cortex-A-Boot-Process.md)

Few interesting questions from todays exploration are captured under [boot.md file](exploration/questions/boot.md)

