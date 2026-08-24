**Core Bringup Stages & Key Concepts**

* **Phase 0: Hardware Power & Clock Stabilization**
* **PMIC Rail Sequencing:** Regulators ramp in strict sequence ($V_{\text{DD\_CORE}} \rightarrow V_{\text{DD\_MEM}} \rightarrow V_{\text{DD\_IO}}$) to avoid CMOS latch-up.

* **Clock Lock & Boot Straps:** Crystals start up, system PLLs lock, and the reset signal (`POR_N`) releases. The hardware samples physical boot pins/straps to determine the initial boot device (eMMC, SPI NOR, PCIe).

* **Phase 1: BootROM (BL1) & Early Trusted Firmware (BL2)**
* **BootROM (BL1):** Hardcoded ROM running at Secure EL3. Validates boot straps, initializes flash storage, and verifies the BL2 cryptographic signature using the Root of Trust (ROTPK).

* **BL2 (SRAM Execution):** Runs entirely inside internal SRAM. Configures early power rails, initializes PLLs, and performs **DDR PHY Calibration** (DQS leveling, $V_{\text{REF}}$ tuning, eye-diagram training). Once DRAM is operational, BL2 loads downstream stages into main memory.

* **Phase 2: Secure Runtime & Non-Secure Bootloader (BL31 / BL33)**
* **BL31 (TF-A Runtime):** Stays resident in Secure EL3 to handle runtime power management (PSCI), system control (SCMI), and Secure Monitor Calls (SMC).

* **BL33 (U-Boot / Barebox):** Runs in Non-Secure EL2/EL1 from main DRAM. Initializes high-level storage, networking, and prepares the CPU environment for kernel handoff.

* **Phase 3: Hardware Description & OS Kernel Handoff**
* **Device Tree (DTB):** Flattened Device Tree binary passes board topology (memory base/size, peripheral MMIO addresses, interrupt lines) to the kernel.

* **Kernel Entry Contract (`head.S`):** Before jumping to the kernel image, the CPU state must satisfy fixed constraints: MMU disabled, D-Cache disabled, interrupts masked, register `x0` pointing to the physical DTB address, and `x1`–`x3` set to 0.

**Essential Bringup Diagnostics & Debug Tools**

| Tool / Technique | Diagnostic Purpose | Common Failure Symptom Resolved |
| --- | --- | --- |
| **Early UART / LL-Debug** | Low-level register-based print routines inside SRAM. | Quiet hangs during early BL2/U-Boot execution before full driver stack setup. |
| **JTAG / SWD Debugger** | Halting CPU core 0 to inspect registers (`SCTLR`, `PC`, `SP`) and RAM. | Infinite loops, exception vectors trapping on undefined instructions or alignment faults. |
| **Logic Analyzer / Scope** | Measuring voltage rail rise times, PLL lock signals, and bus lines (I2C, SPI). | PMIC power-good dropouts, DDR clock instability, missing boot-pin strap levels.|
| **DDR Stress Testing** | Memtester/stressapptest loops post-PHY training. | Intermittent kernel panics or bit flips caused by aggressive DDR timing or thermal drift.|
