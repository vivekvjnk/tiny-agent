### **1. Linked Lists: Lock-Free RCU Unlink Engine**

**Concept:** Intrusive Doubly Linked Lists & Read-Copy-Update (RCU)

**Scenario:** Linux drivers manage dynamic data structures (e.g., loaded kernel modules) using intrusive lists (`struct list_head`). Safe node unlinking while lockless readers traverse the list requires strict memory ordering and RCU grace-period handling.

**Problem:** Implement an intrusive doubly linked list node deletion function in C (`list_del_rcu_safe`).

* The intrusive list uses a standard kernel-style definition:
```c
struct list_head {
    struct list_head *next, *prev;
};

```


* Implement `void list_del_rcu_safe(struct list_head *entry)` to unlink `entry` from a circular doubly linked list without corrupting concurrent lockless readers traversing forward via `next` pointers.


* Ensure proper CPU memory barrier placement (e.g., `WRITE_ONCE` semantics) so that any concurrent reader reading `node->next` never observes an invalid pointer or a broken chain.
* **Constraints:** Do not use heavy spinlocks during traversal; design specifically for reader-side RCU semantics.



---

### **2. Stacks: Zero-Allocation Interrupt Context Frame Stack**

**Concept:** Fixed Storage Stack & Memory Alignment

**Scenario:** In bare-metal or RTOS firmware, handling nested hardware interrupts requires saving interrupt frame states onto a pre-allocated stack without using dynamic allocation (`malloc`).

**Problem:** Design a thread-safe, static interrupt frame stack in C or C++.

* Define a fixed-capacity byte array that acts as the hardware stack buffer.
* Implement `bool push_context_frame(const uint32_t *regs, size_t reg_count, uint32_t align_bytes)` and `bool pop_context_frame(uint32_t *regs, size_t reg_count)`.
* **Rules:**
* Enforce strict memory alignment bounds (e.g., AAPCS requires 8-byte stack alignment at public interfaces).
* Check for stack overflow and underflow strictly without executing unsafe pointer arithmetic.
* Must be reentrant or safe against concurrent calls from different privilege levels (e.g., using atomic operations for the stack pointer).



---

### **3. Queues: Lock-Free Single-Producer Single-Consumer (SPSC) Ring Buffer**

**Concept:** Circular Queue & Hardware Doorbell Pattern

**Scenario:** High-throughput accelerators (e.g., TPUs, NVMe, Ethernet MACs) use host-to-device Submission Queues (SQ) via ring buffers with head/tail index doorbells.

**Problem:** Implement a lock-free SPSC Ring Buffer Queue in C or modern C++.

* Define the ring buffer with power-of-two capacity $N$ to enable fast modulo arithmetic via bitwise masking (`index & (N - 1)`).
* Implement `bool enqueue(const CommandDescriptor *cmd)` (Producer/Host) and `bool dequeue(CommandDescriptor *cmd)` (Consumer/Device).


* **Rules:**
* Do not use mutexes or condition variables.
* Use atomic primitives (`std::atomic` or GCC `__atomic` builtins) with correct memory orderings (`memory_order_release` for updating indices, `memory_order_acquire` when reading indices) to prevent memory reordering issues across CPU cores.



---

### **4. Hash Tables: Static Memory Lock-Free Open Addressing Table**

**Concept:** Open Addressing (Linear Probing) & Dynamic Hardware Identifiers

**Scenario:** Microcontrollers often map device IDs, I2C addresses, or CAN filter IDs to internal driver structures without using runtime dynamic heap memory (`malloc`/`free`).

**Problem:** Design a fixed-size, thread-safe Hash Table in C that maps `uint32_t key` (e.g., CAN Bus Message ID) to `uint32_t value` (e.g., callback function pointer address).

* Use linear probing for collision resolution.
* **Rules:**
* Must run in a fixed static memory footprint (no dynamic allocations).
* Implement atomic insertion (`insert_atomic(key, val)`) and lookup (`lookup(key)`).
* Handle key collisions, table full conditions, and lock-free atomic updates (using Atomic Compare-And-Swap / CAS) so multiple tasks or ISRs can insert key-value pairs concurrently without corruption.



---

### **5. Trees: Device Tree Parent-Child Bus Address Translator**
**Concept:** Flattened Device Tree (FDT) N-ary Tree Traversal & Memory Range Mapping

**Scenario:** Drivers translate bus-local addresses to CPU physical addresses by traversing nested Flattened Device Tree nodes using parent `<ranges>` properties.

**Problem:** Implement a recursive or iterative DTB Node Tree Translation algorithm in C++.

* Represent a DTB bus node as:
```cpp
struct BusNode {
    std::string name;
    std::vector<BusRange> ranges; // {child_base, parent_base, size}[cite: 2]
    std::vector<BusNode*> children;
};

```
* Implement `std::optional<uint64_t> translate_address(const BusNode* leaf_node, uint64_t child_bus_addr)`.
* **Rules:**
* Walk up the tree node path from `leaf_node` to the root node (CPU Physical Space).

* At each level, translate `child_bus_addr` to `parent_base` using matching range windows:

$$\text{Parent Addr} = \text{parent\_base} + (\text{child\_addr} - \text{child\_base}) \quad \text{[cite: 2]}$$

* If a parent node has an empty `ranges` vector, assume 1:1 identity translation. Return `std::nullopt` if the address falls outside declared range bounds.