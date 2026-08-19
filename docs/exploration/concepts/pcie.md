# Peripheral Component Interface Express (PCIe) Architecture (Point-to-Point Serial Interconnect)

PCIe completely abandons the classic shared parallel bus paradigm in favor of a **switched, point-to-point, packetized serial topology**.

```
[ Classical Parallel Bus ]             [ PCIe Switched Topology ]

CPU ---+--- Memory                      CPU / Root Complex
       |                                      |
       +--- Device A (Shared Lines)      +----+----+  (Dedicated Point-to-Point
       |                                 |         |   Dual-Simplex Links)
       +--- Device B (Shared Lines)   Device A  Device B

```

---

## 1. Shared Parallel Bus vs. PCIe Point-to-Point

| Feature | Shared Parallel Bus (PCI/PCI-X) | PCIe Point-to-Point |
| --- | --- | --- |
| **Physical Topology** | Shared multi-drop tracks | Switched, dedicated point-to-point lanes |
| **Electrical Interface** | Single-ended, shared bidirectional lines | Low-Voltage Differential Signaling (LVDS) |
| **Bus Logic** | Requires Tri-State / High-Z & Turnaround | **No Tri-State** (Dedicated Tx and Rx pairs) |
| **Clocking** | Dedicated parallel clock wire (Subject to **Clock Skew**) | Embedded clock in data stream via CDR (Clock & Data Recovery) |
| **Transfer Unit** | Voltage levels mapped to parallel word lines | Serialization of **TLPs** (Transaction Layer Packets) |
| **Arbitration** | Centralized bus arbiter (Grant/Request lines) | Distributed link-level packet flow control (Credit-based) |

---

## 2. Physical Lane Anatomy (Eliminating High-Z)

A single PCIe lane consists of **4 physical wires** split into two independent unidirectional differential pairs (Full-Duplex / Dual-Simplex):

```
       +------------------- Lane (x1) -------------------+
       |                                                 |
       |   Transmitter  +---------------+  Receiver      |
[Node A] -- (Tx+ / Tx-) |======>>=======| (Rx+ / Rx-) --> [Node B]
       |                +---------------+                |
       |   Receiver     +---------------+  Transmitter   |
[Node A] <-- (Rx+ / Rx-) |======<<=======| (Tx+ / Tx-) <-- [Node B]
       |                +---------------+                |
       +-------------------------------------------------+

```

* **No Bus Contention:** Node A and Node B never fight over line driving. Node A *only* drives its Tx pair; Node B *only* drives its Tx pair.
* **No Turnaround Cycles:** Devices can transmit and receive simultaneously without waiting for bus settling time.
* **Differential Noise Immunity:** Signal is measured as $V_{diff} = (V_+) - (V_-)$. Common-mode electromagnetic interference (EMI) affects both lines equally, cancelling out noise and allowing GHz-range clock speeds.

---

## 3. The PCIe Protocol Layer Stack

Transactions are no longer driven as raw address voltage pulses; they are wrapped into network-like packets:

```
+-----------------------------------------------------------------+
| Transaction Layer   | Builds TLPs (Memory Read/Write, IO, MSI)  |
+-----------------------------------------------------------------+
| Data Link Layer     | ACK/NAK, Sequence Numbers, Flow Control   |
+-----------------------------------------------------------------+
| Physical Layer (PHY)| Encoding (128b/130b), Serialization, CDR  |
+-----------------------------------------------------------------+

```
