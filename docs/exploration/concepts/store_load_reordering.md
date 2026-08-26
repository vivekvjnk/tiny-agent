We need to separate **sequential execution semantics inside a single CPU thread** from **how memory stores actually become visible across the rest of the system** (other CPU cores, DMA engines, or PCIe devices).

The mental model of the Von Neumann *load-operate-store* procedure is correct, but modern processors do **not** execute programs as a simple sequential pipeline that immediately writes directly to main RAM.

---

### **1. Single-Threaded Illusion vs. Multithreaded Reality**

Inside a single CPU core, the processor enforces the **Program Order Illusion** (also called *as-if* execution).

* The compiler and hardware CPU are permitted to reorder instructions, execute them out-of-order, or group memory access operations **as long as single-threaded results remain identical**.
* **The CPU Catch:** The CPU *only* tracks dependencies within its own instruction stream. It has zero visibility into what *other* CPU cores or hardware devices (like PCIe controllers) are expecting in shared RAM.



---

### **2. How Can a Store/Write Be Reordered? (The Hardware Store Buffer)**

You asked: *"In case of store, the update should be done in CPU first then the value should be stored. How is it possible to move write or read before store or load?"*

Here is how the hardware actually handles a `Store` instruction:

```
[ CPU Core ] ──(1. Fast Store)──> [ Store Buffer ] ──(2. Delayed Drain)──> [ L1/L2 Cache / RAM ]

```
[Refer](fast_store_delayed_drain.md)

1. **The Store Execution:** When the CPU executes a `Store` instruction (e.g., writing data payload to a buffer), it doesn't wait for the data to travel across the bus to RAM. It places the data into a ultra-fast local hardware structure called a **Store Buffer** (or Write Buffer) and immediately marks the instruction as "complete."


2. **The Reordering Hazard:** The CPU core continues executing subsequent instructions. If the next instruction is another `Store` (e.g., updating the `tail` pointer or ringing a PCIe Doorbell), that second store might drain from the Store Buffer to main RAM **before** the first store finishes draining!



**To the local CPU thread:** Everything happened in order.

**To another CPU core or PCIe device:** The `tail` pointer or doorbell write appeared in RAM **before** the payload data was written!

---

### **3. How Can a Load/Read Be Reordered? (**Speculative Execution** & Invalidation Queues)**

A CPU core reads data from its local L1/L2 cache lines. If a core wants to read a pointer or array element:

1. **Speculative Fetching:** Out-of-order cores speculatively fetch and load data from L1 cache lines early to avoid stalling on slow memory access.
2. **Invalidation Queues:** If Core 0 updates a variable, it sends a Cache Invalidation Message to Core 1. Core 1 places this message in an **Invalidation Queue** to process later.
3. If Core 1 executes a `Load` before processing its Invalidation Queue, it will read **stale data** out of its L1 cache—effectively reordering the read *before* the remote write actually became visible.

---

### **4. Real-World Firmware Example: The Doorbell Trap**

Consider the classic PCIe Doorbell pattern used in DMA and accelerator drivers:

```c
// Step 1: Write payload descriptor into shared RAM
ring_buffer[tail].cmd = COMPUTE_MATRIX; 

// Step 2: Ring hardware doorbell register (Tell accelerator to execute)
*doorbell_register = tail + 1; 

```

#### **Without Memory Barriers (What goes wrong):**

1. Compiler or CPU puts `Step 1` in the Store Buffer.
2. CPU executes `Step 2` (writing to the PCIe MMIO mapped register).

3. The PCIe Doorbell hardware receives the register update instantly.

4. The PCIe accelerator uses DMA to fetch `ring_buffer[tail]` from RAM.

5. **CRASH / CORRUPTION:** The CPU Store Buffer hasn't finished flushing `Step 1` to RAM yet! The accelerator reads uninitialized garbage out of RAM.

---

### **5. Summary: What Barriers & Atomic Orderings Actually Do**

When you use memory barriers (`smp_wmb()`, `std::memory_order_release`, or `std::memory_order_acquire`):

* **`memory_order_release` (Store Barrier):** Forces the CPU to completely flush its local **Store Buffer** to the coherent cache/RAM before allowing any subsequent stores (like setting a flag or updating `tail`) to execute.


* **`memory_order_acquire` (Load Barrier):** Forces the CPU to clear/process its **Invalidation Queues** and invalidates local cache lines before executing subsequent reads, ensuring it fetches the freshest data from RAM.