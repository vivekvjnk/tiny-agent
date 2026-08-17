### BL2 (Trusted Boot Firmware) Technical Summary

#### 1. System Execution & Memory Topology

```
+-----------------------------------------------------------------------------------+
| INTERNAL ON-CHIP SRAM                                                             |
|                                                                                   |
|  +--------------------+      Loads & Verifies      +--------------------------+   |
|  | BL1 (Secure EL3)   | -------------------------> |    BL2 (Secure EL3/EL1)  |   |
|  +--------------------+                            +--------------------------+   |
|                                                                 |                 |
+-----------------------------------------------------------------|-----------------+
                                                                  | Loads binaries to DRAM
                                                                  v
+-----------------------------------------------------------------------------------+
| MAIN PHYSICAL DRAM                                                                |
|                                                                                   |
|  +--------------------+      +--------------------+      +---------------------+  |
|  | BL31 (Runtime EL3) |      | BL32 (OP-TEE SEL1) |      | BL33 (U-Boot EL2)   |  |
|  +--------------------+      +--------------------+      +---------------------+  |
+-----------------------------------------------------------------------------------+

```

#### 2. Architectural Specification

| Attribute | Specification / Firmware Detail |
| --- | --- |
| **Execution Context** | Runs out of small, reliable **On-Chip SRAM** (typically 128 KB – 512 KB).

 |
| **Privilege Level** | Secure EL3 or Secure EL1 (AArch64 ARM Trusted Firmware).

 |
| **HW State at Entry** | MMU & D-Caches disabled; Clocks & PMIC core rails stabilized.

 |
| **Primary Mandates** | <br>**1.** DDR PHY Initialization & Training (Read/Write leveling, $V_{\text{REF}}$ calibration) .

<br>

<br>**2.** Secure Boot Authentication (Validates RSA/ECC signatures against ROTPK) .

<br>

<br>**3.** Downstream Binary Loading (Fetches BL31, BL32, BL33 into DRAM).

 |
| **Handoff Target** | Transfers control back to **BL31** (Secure EL3 Runtime Monitor).

 |

---

#### 3. Core Execution Pipeline

```
 [BL2 Entry]
      │
      ▼
 ┌─────────────┐
 │ Peripheral  │ ──> Configures system PLL clocks, PMIC rails, and primary boot 
 [cite_start]│    Init     │     storage controller (eMMC / SPI Flash / PCIe)[cite: 719, 813].
 └─────────────┘
      │
      ▼
 ┌─────────────┐
 │  DDR PHY    │ ──> Executes DQS read/write leveling, setup/hold adjustments, and 
 [cite_start]│ Calibration │     VREF tuning to make main DRAM operational[cite: 741].
 └─────────────┘
      │
      ▼
 ┌─────────────┐
 │ Authenticate│ ──> Verifies cryptographic signatures of BL31, BL32, and BL33 binaries 
 │   & Load    │     against hardware Root of Trust. [cite_start]Maps binaries into DRAM[cite: 820].
 └─────────────┘
      │
      ▼
 [BL31 Handoff]

```
