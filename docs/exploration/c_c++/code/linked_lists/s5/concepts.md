
In the Linux kernel and embedded systems, hardware is described using a **Device Tree**. Because hardware is organized in nested buses (e.g., CPU $\rightarrow$ PCIe Host Controller $\rightarrow$ PCIe Switch $\rightarrow$ Endpoint), physical addresses used by devices inside a child bus often do not match the address space of the main CPU memory system.

The `<ranges>` property in a Device Tree translates address spaces between child buses and parent buses as you walk up the tree toward the root CPU bus.

---

### Key Concepts

**1. Address Domains & Translation**

* **Child Address Space:** The address a specific device on a sub-bus listens to.
* **Parent Address Space:** The address space of the bus directly above it.
* **Translation:** Mapping a local offset from the child bus into the parent's window until you reach the root (which represents true CPU Physical Addresses).

**2. The `<ranges>` Translation Rule**
Each entry in a node's `ranges` vector acts as a mapping window:


$$\text{Range Entry} = \{\text{child\_base}, \text{parent\_base}, \text{size}\}$$

For an address `child_addr` to be valid for a range entry, it must satisfy:


$$\text{child\_base} \le \text{child\_addr} < \text{child\_base} + \text{size}$$

When a valid window is found, the transformed parent address is calculated as:


$$\text{Parent Addr} = \text{parent\_base} + (\text{child\_addr} - \text{child\_base})$$

**3. Edge Case: Empty Ranges (1:1 Translation)**
If a bus node has an **empty** `ranges` list (or in DT terms, `ranges;`), it signifies an **identity mapping**. The child address space is directly mapped 1:1 to the parent address space without offset modifications:


$$\text{Parent Addr} = \text{child\_addr}$$

---

### Step-by-Step Translation Walkthrough

Imagine this tree topology:

* **Root (CPU Space)**
* **PCIe Host Controller** (`ranges`: `{child: 0x80000000, parent: 0x40000000, size: 0x10000000}`)
* **PCI Express Switch** (`ranges`: `{child: 0x0000, parent: 0x80000000, size: 0x1000}`)
* **Sensor Device** (Leaf Node)







Translating local address `0x0100` from the **Sensor Device**:

```
[Leaf Node: Sensor]  Local Addr = 0x0100
        │
        ▼  Check PCI Express Switch ranges:
           0x0000 <= 0x0100 < 0x1000  (Matches window)
           New Addr = 0x80000000 + (0x0100 - 0x0000) = 0x80000100
        │
        ▼  Check PCIe Host Controller ranges:
           0x80000000 <= 0x80000100 < 0x90000000  (Matches window)
           New Addr = 0x40000000 + (0x80000100 - 0x80000000) = 0x40000100
        │
        ▼  Root Node Reached!
[CPU Physical Addr] = 0x40000100

```

