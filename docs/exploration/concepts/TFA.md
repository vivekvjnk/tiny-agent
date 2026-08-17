# Arm Trusted Firmware-A (TF-A) Architecture & Core Services

Arm Trusted Firmware-A (TF-A) provides the reference implementation of Secure World software for Armv8-A and Armv9-A architectures. It operates at **EL3 (Secure Monitor)** and provides runtime services to Non-Secure operating systems via the **SMC (Secure Monitor Call)** interface.

---

## 1. Boot Stages & Execution Topology

```
+-----------------------------------------------------------------------------------------+
| SECURE WORLD                                    | NON-SECURE WORLD                      |
+-------------------------------------------------+---------------------------------------+
|                                                 |                                       |
|  +--------------------+                         |                                       |
|  |   BL1 (Boot ROM)   |  (EL3)                  |                                       |
|  +--------------------+                         |                                       |
|            |                                    |                                       |
|            v                                    |                                       |
|  +--------------------+                         |                                       |
|  |  BL2 (SRAM Firmware)| (EL3/EL1)               |                                       |
|  +--------------------+                         |                                       |
|            |                                    |                                       |
|            +-------------------+----------------+-----------+                           |
|            |                   |                            |                           |
|            v                   v                            v                           |
|  +------------------+  +------------------+       +-------------------+                 |
|  | BL31 (Runtime)   |  | BL32 (OP-TEE)    |       | BL33 (U-Boot)     |                 |
|  | EL3 Monitor      |  | Secure EL1       |       | Non-Secure EL2    |                 |
|  +------------------+  +------------------+       +-------------------+                 |
|            ^                                                |                           |
|            |------------- SMC Exception Call ---------------|                           |
|            v                                                v                           |
|  +------------------+                             +-------------------+                 |
|  | PSCI / SCMI      |                             | Linux Kernel      |                 |
|  | Handlers         |                             | Non-Secure EL1    |                 |
|  +------------------+                             +-------------------+                 |
+-----------------------------------------------------------------------------------------+
```

---

## 2. Component Reference Table

| Boot Stage | Name / Role | Exception Level | Execution Memory | Primary Responsibilities |
| --- | --- | --- | --- | --- |
| **BL1** | AP Boot ROM | EL3 (Secure) | On-Chip ROM | Primary HW initialization, public key validation, loads BL2 into SRAM. |
| **BL2** | Trusted Boot Firmware | EL3 / S-EL1 | On-Chip SRAM | DDR PHY initialization, authenticates and loads BL31, BL32, BL33. |
| **BL31** | EL3 Runtime Monitor | EL3 (Secure) | Secure SRAM / DRAM | Remains resident in memory. Handles SMC calls, PSCI, SCMI, exception routing. |
| **BL32** | Secure Payload (OP-TEE) | Secure EL1 | Secure DRAM (T TrustZone) | Handles trusted applications, DRM, cryptographic keys, hardware security modules. |
| **BL33** | Non-Secure Bootloader | NS-EL2 | Main DRAM | U-Boot / Barebox. Prepares DTB, initializes primary drivers, boots Linux kernel. |

---

## 3. ARM SMC Calling Convention (SMCCC)

Non-Secure OS calls TF-A using the `smc #0` instruction. Registers `x0`-`x7` pass arguments and return values.

### `x0` Function Identifier Bitfield

| Bits | Field | Description |
| --- | --- | --- |
| `[31]` | Fast / Yielding Call | `1` = Fast Call (Atomic, non-preemptible), `0` = Yielding Call. |
| `[30]` | Calling Convention | `1` = SMC64 (64-bit parameters), `0` = SMC32 (32-bit parameters). |
| `[29:24]` | Service Owner ID | `0x00` = ARM Architecture, `0x02` = CPU Service (PSCI), `0x04` = SiP Service, `0x06` = SCMI. |
| `[23:16]` | Reserved | MBZ (Must Be Zero). |
| `[15:0]` | Function Number | Specific API command ID. |

---

## 4. Power State Coordination Interface (PSCI)

PSCI provides standard CPU lifecycle management for OS kernels via TF-A EL3 handlers.

| PSCI Function | SMC64 Function ID | Parameters | Return Value / Behavior |
| --- | --- | --- | --- |
| `PSCI_VERSION` | `0x84000000` | None | `[31:16]` Major, `[15:0]` Minor version. |
| `CPU_ON` | `0xC4000003` | `target_cpu` (`MPIDR`), `entry_point_address`, `context_id` | Boots target secondary core at target address. Returns `0` on success. |
| `CPU_OFF` | `0x84000002` | None | Powers down invoking core. Does not return. |
| `CPU_SUSPEND` | `0xC4000001` | `power_state`, `entry_point_address`, `context_id` | Enters low-power state (Retention / Powerdown). Woken by IRQ/IPI. |
| `SYSTEM_OFF` | `0x84000008` | None | Powers off total board via PMIC interface. |
| `SYSTEM_RESET` | `0x84000009` | None | Triggers full cold system reset. |
| `SYSTEM_SUSPEND` | `0xC400000E` | `entry_point_address`, `context_id` | Suspends system-on-chip to low power state (RAM retention). |

---

## 5. System Control & Management Interface (SCMI)

SCMI offloads system power, clock, thermal, and sensor management to a dedicated System Control Processor (SCP) or TF-A runtime monitor using shared memory queues and doorbells.

```
+--------------------------+                         +--------------------------+
| Linux Kernel (NS-EL1)    |                         | TF-A / SCP (Secure/Firmware)
|                          |                         |                          |
|  +--------------------+  |                         |  +--------------------+  |
|  | Clock/Power Driver |  |                         |  | SCMI Server Module|  |
|  +--------------------+  |                         |  +--------------------+  |
|            |             |                         |            ^             |
|  Builds Message Packet   |                         |   Parses & Executes      |
|            v             |                         |            |             |
|  +--------------------+  |    Shared SRAM Buffer   |  +--------------------+  |
|  | Mailbox / Shared   |==|=========================|==| Mailbox IRQ / SMC  |  |
|  | Memory Transport   |  | [Header][Payload][Resp] |  | Interrupt Handler  |  |
|  +--------------------+  |                         |  +--------------------+  |
+--------------------------+                         +--------------------------+
```

### SCMI Protocol Domains

| Protocol ID | Domain | Capabilities / Operations |
| --- | --- | --- |
| `0x10` | Base Protocol | Query architecture version, agent count, supported protocols. |
| `0x11` | Power Domain | Query and set power states (On, Off, Sleep) for power islands. |
| `0x12` | System Power | Manage system-level power transitions (Shutdown, Cold/Warm Reset). |
| `0x13` | Performance (DVFS) | Set CPU/GPU operating performance points (OPPs), frequency, voltage scaling. |
| `0x14` | Clock | Enable/disable system clocks, query rates, set rate configurations. |
| `0x15` | Sensor | Read system hardware telemetry (Temperature, Voltage, Current, Power). |
| `0x16` | Reset Domain | Assert and de-assert hardware reset lines for IP cores. |
