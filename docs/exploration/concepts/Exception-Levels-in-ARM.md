# ARMv8/ARMv9 Exception Levels & Security Architecture

The Armv8-A and Armv9-A architectures define a hierarchical execution privilege model organized into **Exception Levels (EL0 to EL3)**, orthogonal to **Security Worlds** (Secure, Non-Secure, Realm, Root).

---

## 1. Exception Levels & Privilege Topology

```
 +-------------------------------------------------------------------------------------------------+
 | PRIVILEGE   | NON-SECURE WORLD       | SECURE WORLD           | REALM WORLD (Armv9 RME) | ROOT  |
 +-------------+------------------------+------------------------+-------------------------+-------+
 | EL3         |                        |                        |                         | TF-A  |
 | (Monitor)   |                        |                        |                         | Root  |
 +-------------+------------------------+------------------------+-------------------------+-------+
 | EL2         | Hypervisor             | Secure Partition       | Realm Management System |       |
 | (Hypervisor)| (KVM / Xen)            | Manager (SPMC)         | (RMM)                   |       |
 +-------------+------------------------+------------------------+-------------------------+-------+
 | EL1         | OS Kernel              | Secure OS              | Realm OS                |       |
 | (Kernel)    | (Linux / Windows)      | (OP-TEE)               | Kernel                  |       |
 +-------------+------------------------+------------------------+-------------------------+-------+
 | EL0         | User Applications      | Trusted Applications   | Realm Applications      |       |
 | (User)      | (Bash, Chrome)         | (TA / Enclaves)        |                         |       |
 +-------------------------------------------------------------------------------------------------+
```

---

## 2. Privilege Level Summary Matrix

| Exception Level | Typical Software Component | Privileges & Capabilities | Transition Instruction |
| --- | --- | --- | --- |
| **EL0** | User Space Applications | Unprivileged execution; restricted access to hardware registers and system memory. | `SVC` (Supervisor Call to EL1) |
| **EL1** | Operating System Kernel | Privileged OS kernel space; virtual memory management, device drivers, process isolation. | `HVC` (Hypervisor Call to EL2) |
| **EL2** | Hypervisor / Virtualization | Virtualization control, Stage-2 MMU address translation, guest VM trapping. | `SMC` (Secure Monitor Call to EL3) |
| **EL3** | Firmware / Secure Monitor | Highest hardware privilege; controls world switching, PSCI power handling, TrustZone config. | `ERET` (Exception Return to lower EL) |

---

## 3. Key Architectural Control Registers

| Register | Exception Level | Primary Function & Bit Control |
| --- | --- | --- |
| **`SCR_EL3`** | EL3 | **Secure Configuration Register:** Controls NS (Non-Secure bit), HCE (HVC Enable), SMD (SMC Disable), EEL2 (Secure EL2 enable). |
| **`HCR_EL2`** | EL2 | **Hypervisor Control Register:** Controls VM (Stage-2 MMU enable), TGE (Trap General Exceptions), IMO/FMO (IRQ/FIQ routing to EL2). |
| **`SCTLR_EL1`** | EL1 | **System Control Register:** Controls M (MMU enable), C (Data Cache enable), I (Instruction Cache enable), A (Alignment Check). |
| **`VBAR_ELx`** | EL1 / EL2 / EL3 | **Vector Base Address Register:** Holds base address of exception vector table for the target EL. |
| **`PSTATE`** | All | **Process State:** Holds condition flags (`N`, `Z`, `C`, `V`), execution state (`D`, `A`, `I`, `F` interrupt masks), and current EL. |

---

## 4. Exception Vector Table Layout (`VBAR_ELx`)

Each EL has its own 2KB-aligned Exception Vector Table containing 16 entry slots (128 bytes each, 32 instructions):

```
 Base Address (VBAR_ELx)
   │
   ├── +0x000 : Current EL with SP0 ------------ Synchronous Exception
   ├── +0x080 : Current EL with SP0 ------------ IRQ / vIRQ
   ├── +0x100 : Current EL with SP0 ------------ FIQ / vFIQ
   ├── +0x180 : Current EL with SP0 ------------ SError / vSError
   │
   ├── +0x200 : Current EL with SPx ------------ Synchronous Exception
   ├── +0x280 : Current EL with SPx ------------ IRQ / vIRQ
   ├── +0x300 : Current EL with SPx ------------ FIQ / vFIQ
   ├── +0x380 : Current EL with SPx ------------ SError / vSError
   │
   ├── +0x400 : Lower EL using AArch64 --------- Synchronous Exception (e.g., SVC/HVC/SMC)
   ├── +0x480 : Lower EL using AArch64 --------- IRQ / vIRQ
   ├── +0x500 : Lower EL using AArch64 --------- FIQ / vFIQ
   ├── +0x580 : Lower EL using AArch64 --------- SError / vSError
   │
   ├── +0x600 : Lower EL using AArch32 --------- Synchronous Exception
   ├── +0x680 : Lower EL using AArch32 --------- IRQ / vIRQ
   ├── +0x700 : Lower EL using AArch32 --------- FIQ / vFIQ
   └── +0x780 : Lower EL using AArch32 --------- SError / vSError
```

---

## 5. Central Security Architecture (e.g. i.MX8 / TrustZone)

Silicon vendors enforce EL security partitioning through bus-level hardware controllers:

```
               +--------------------------------------------------+
               | Master Devices (Core, DMA, PCIe)                 |
               | Drives AXI Bus Transaction with AxPROT[1] (NS)   |
               +--------------------------------------------------+
                                        │
                                        ▼
               +--------------------------------------------------+
               | Central Security Unit (CSU) / TZASC / SMMU        |
               | Compares AxPROT[1] vs Configured Access Matrix   |
               +--------------------------------------------------+
                                        │
                    ┌───────────────────┴───────────────────┐
                    │                                       │
                    ▼                                       ▼
       [NS Bit = 0 (Secure Pass)]             [NS Bit = 1 (Access Denied)]
                    │                                       │
                    v                                       v
       +-------------------------+             +-------------------------+
       | Secure SRAM / Registers |             | Triggers AXI Bus Fault  |
       +-------------------------+             | or DECERR / SError      |
                                               +-------------------------+
```
