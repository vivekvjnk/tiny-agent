# DDR peripheral

## 1. DDR Control registers
1. address and IO control
    - Clock IO pad
        - slew rate of clock IO pads
        - pull up/pull down output impedence of clock IO pas
    - Program addr/cmd pad
        - addr/cmd IO pads output slew rate 
        - program addr/cmd IO pad pull up/pull down output impedence
2. Data IO control
    - WD0 & WD1 controls
        - DQS0/1/2
        - DM0/1/2
        - DDR data pins
    - Clock IO pads
        - DQS(0/1/2) or DQ(0/1/2) output slew rate 
        - DQS(0/1/2) or DQ(0/1/2) pullup/pulldown impedence
    - Data IO pads
        - output slew rate
        - pull up/pull down impedence
3. DDR IO control
## 2. EMIF configuration registers
- structure to hold DDR EMIF registers 

```c
typedef struct sblPlatformDdrEmifCfg{
    uint32_t ddrPhyCtrl;
    // DDR phy control register
    uint32_t ddrPhyCtrlDyPwrdn
    // DDR phy dynamic power down register value
    uint32_t sdramTim1; 
    // SDRAM timing 1 register
    uint32_t sdramTim2;
    // SDRAM timing 2 register
    uint32_t sdramTim3;
    // SDRAM timing 3 register
    uint32_t sdramConfig;
    // SDRAM configuration register 
    uint32_t sdramRefCtrl1;
    // SDRAM reference control register 1
    uint32_t sdramRefCtrl2;
    // SDRAM reference control register 2
    uint32_t zqConfig;
    // ZQ configuraton register 
}sblPlatformDdrEmitCfg_t;
```

- structure to hold DDR EMIF phy registers
```c
typedef struct sblPlatformDdrEmifPhyCfg{
    /*COMMAND*/
    uint32_t slaveRatio;
    // slave ratio
    uint32_t slaveForce;
    uint32_t slaveDelay;
    uint32_t invertClkout;

    /*DATA*/
    uint32_t rdDqsSlaveRatio;
    // Read DQS slave ratio
    uint32_t wrDqsSlaveRatio;
    // Write DQS slave ratio
    uint32_t fifoWeSlaveRatio;
    // FIFO write enable slave ratio
    uint32_t wrDataSlaveRatio;
    // Writer data slave ratio

}sblPlatformDdrEmifPhyCfg_t;
```

- structure for holding ddr configurations
```c
typedef struct sblPlatformDdrCfg{
    sblPlatformDdrCtrlCfg_t ddrCtrlCfg;
    // DDR control configuration registers
    sblPlatformDdrEmifCfg_t ddrEmifCfg;
    // DDR EMIF configuration registers
    sblPlatformDdrEmifPhyCfg_t ddrEmifPhyCfg;
    // DDR EMIF Phy configuration registers
}
```


# Relay configuration
*Following are the set of DDR configurations in the relay we worked on*

- DDR3 SRAM : AS4C64M16D3LB
    - 1GB SRAM 
    - 1.35V 
    - 800MHz
    - Differential clock    
        - CK & CK#
    - Bidirectional Differential data strobe
        - DQS and DQS#
    - ZQ Calibration
    - Programmable burst lengths : 4,8
- Used configuration similar to Beaglebone Black board

## PLL configuration
- DPLL multiplier = 50
- DPLL Divisor = 2
- DPLL Post Divisor M2 = 1
- DPLL Post Divisor M4 = 1

- PLL0 frequency is set to 300MHz for ARM Cortex A8 core

### Steps to initialize PLL
- Put PLL in bypass mode
    - Wait until PLL is in bypass mode
- Set multiplier and divisor values for the PLL 
- Set clock out 2 divider
- Lock PLL by enabling it
    - Wait until PLL is locked

NOTE: There are dedicated PLL initilization functions for initializing DDR PLL, MPU PLL, Display PLL, Core PLL. These are not separate PLL modules. Instead same PLL from PRCM module can be configured to generate different clocks for each of these modules 



# Miscellaneous
## Slew rate 
* Rise/Fall Time (`t_r / t_f`): This is traditionally measured between the 10% and 90% points (or sometimes 20% to 80%) of the full voltage swing. This measurement ignores the slow, curved "tails" at the very beginning and end of the transition.
* Slew Rate ($dV/dt$): This measures the slope of the steepest, most linear portion of that transition.

Therefore, if an I/O pad transitions from $0V$ to $1.2V$, the mathematical relationship is:

$\text{Slew Rate} \approx \frac{V_{90\%} - V_{10\%}}{t_r} = \frac{0.8 \times \Delta V}{t_r}$

### Why I/O Pads Have Programmable Slew Rates
Many modern digital I/O pads allow you to select "fast" or "slow" slew rate settings because of a critical trade-off:

* Fast Slew Rate (Short $t_r$): Necessary for high-frequency interfaces (like DDR memory) to maintain clean data windows, but it generates significant electromagnetic interference (EMI) and ground bounce (simultaneous switching noise).
* Slow Slew Rate (Long $t_r$): Ideal for lower-speed signals (like I2C or GPIO). It minimizes noise, reduces reflections, and saves power, but it limits the maximum operational frequency.


# DDR Bring up

Integrating the **Alliance Memory AS4C64M16D3LB** (1Gb / 64M x 16 Low-Voltage DDR3L SDRAM) into an SoC requires bridging raw memory silicon specs with early boot firmware setup.

step-by-step roadmap:

### Step 1: Decode Chip Specifications (Datasheet Audit)

Extract the critical parameters from the AS4C64M16D3LB datasheet needed to program your SoC's **EMIF/DDR Controller**:

* **Bus Topology:** 16-bit wide data bus ($DQ[15:0]$), 1 Gb total capacity ($64\text{M} \times 16$).
* **Voltage ($V_{\text{DD}} / V_{\text{DDQ}}$):** Operates at $1.35\text{ V}$ (DDR3L backwards compatible with $1.5\text{ V}$ DDR3).
* **Addressing Architecture:**
* 8 Bank Addresses ($BA[2:0]$)
* 13 Row Addresses ($A[12:0]$)
* 10 Column Addresses ($A[9:0]$)


* **Speed Grade & JEDEC Timings:** Match your target clock frequency (e.g., 400 MHz / 800 MT/s or 533 MHz / 1066 MT/s) to obtain key AC timing parameters in nanoseconds/cycles: $t_{\text{RCD}}$, $t_{\text{RP}}$, $t_{\text{RAS}}$, $t_{\text{RFC}}$, and $t_{\text{REFI}}$ (typically $7.8\,\mu\text{s}$ at standard operational temperatures).

---

### Step 2: System & Power-On Initialization Sequence

Ensure the early bootloader (e.g., BootROM or SPL/BL2 executing out of **Internal SRAM**) powers up the memory interface following strict JEDEC timing rules:

1. **Power Rail Sequence:** Ramp $V_{\text{DD}}$ and $V_{\text{DDQ}}$ rails concurrently while asserting `RESET#` (active low).


2. **Clock Stabilization:** Enable the differential clock inputs ($CK / CK\#$) and maintain `RESET#` low for at least $200\,\mu\text{s}$.
3. **CKE Assertion:** Drive Clock Enable (`CKE`) HIGH at least $500\,\mu\text{s}$ after power-up.
4. **MR (Mode Register) Initialization Sequence:** Issue sequential `MRS` commands via the address bus:
* **MR2:** Configures CAS Write Latency (CWL) and dynamic ODT options.
* **MR3:** Configures MPR functions.
* **MR1:** Enables DLL, configures Output Drive Impedance (e.g., $34\,\Omega$) and On-Die Termination (ODT $R_{TT\_NOM}$).
* **MR0:** Triggers DLL Reset and sets CAS Latency (CL), Read Burst Type, and Write Recovery ($t_{WR}$).


5. **ZQ Calibration:** Execute a `ZQCL` (ZQ Calibration Long) command to calibrate output impedance and termination values against external pull-down resistors.

---

### Step 3: Configure Controller Register Model

Map the memory specifications into your SoC's DDR Memory Controller (e.g., TI EMIF4D) registers:

| Register Target | Configuration Focus | Functional Impact |
| --- | --- | --- |
| **Geometry Registers** | Bus width (16-bit), Bank count (8), Row bits (13), Column bits (10) | Establishes physical-to-virtual memory mapping space. |
| **Timing Registers** | Program raw cycles for $t_{\text{RCD}}$, $t_{\text{RP}}$, $t_{\text{RAS}}$, $t_{\text{RC}}$, $t_{\text{FAW}}$, $t_{\text{RRD}}$ | Ensures command setup and hold times prevent bus collisions. |
| **Refresh Control** | Program $t_{\text{REFI}}$ based on SoC clock divider | Prevents bit flips by maintaining capacitive charge levels. |
| **Mode Registers** | Write pre-calculated hex masks for MR0–MR3 | Sets DRAM internal state machine to match PHY operation. |

---

### Step 4: DDR PHY Calibration & Training (Boot Stage: SRAM / BL2)

Because multi-megabyte payloads like U-Boot or Linux require functional DRAM, training must run from on-chip SRAM:

* **DQS Read/Write Leveling:** Aligns data strobe edges ($DQS$) with the memory clock ($CK$) to compensate for trace-length differences (fly-by topology).


* **$V_{\text{REF}}$ & Eye-Diagram Training:** Sweeps receiver voltage reference levels and fine-tunes data line delay lines to center sampling points within the valid data eye.



---

### Step 5: Device Tree & Memory Map Integration

Update your board's devicetree (`.dtb`) to expose the 1 Gb ($128\text{ MB}$) region to the OS kernel:

```dts
/ {
    memory@80000000 {
        device_type = "memory";
        reg = <0x80000000 0x08000000>; /* 128MB starting at base address 0x80000000 */
    };
};

```

---

### Step 6: Hardware Validation & Diagnostic Suite

Before booting the OS, run low-level memory tests inside your bootloader (e.g., U-Boot `mtest`):

* **Walking 1s/0s Test:** Identifies shorted or open PCB traces on $DQ[15:0]$ and $A[12:0]$.
* **Stress & Pattern Test:** Runs pseudo-random patterns across the entire 128 MB space to detect thermal stability issues, voltage droops, or incorrect refresh rate setups.
