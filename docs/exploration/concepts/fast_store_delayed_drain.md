### **1. What is a "Store"?**

In low-level CPU assembly (like ARM or x86), a **Store** instruction (e.g., `STR` or `MOV`) takes data from a CPU register and writes it to a memory address.

---

### **2. What is a "Fast Store"?**

When a CPU core executes a store instruction, reaching the physical RAM (or even the shared L3 cache) takes dozens or hundreds of CPU clock cycles. If the CPU core had to stop and wait for that write to actually complete all the way in DRAM, system performance would plummet.

To make stores "fast," modern CPUs place a dedicated hardware buffer right next to the core called a **Store Buffer** (or Write Buffer).

* **The Fast Store Action:** When the CPU executes `store data -> address`, it immediately writes the data into its local **Store Buffer** in **1 single clock cycle**.


* To the executing CPU core, the instruction is officially "complete," so it moves instantly to the next instruction without waiting for RAM.



---

### **3. What is a "Drain" (and why is it "Delayed")?**

A **Drain** is the process where the hardware automatically takes data sitting inside the local Store Buffer and flushes/writes it out to the main cache lines and physical RAM.

#### **Why is it "Delayed"?**

Because the Store Buffer acts like an intermediate waiting room:

1. **Asynchronous Flushing:** The hardware drains the Store Buffer to RAM in the background whenever the internal memory bus is free.


2. **Merging & Reordering:** To optimize bus traffic, the CPU might hold onto stores in the buffer to merge multiple small writes into a single larger write, or reorder the drain sequence based on cache line availability.



---

### **Summary of the Problem in Firmware**

Because of the **fast store + delayed drain** design:

1. **Inside the Local Core:** Everything looks sequential. If the same core reads that address right back, it gets the data directly out of its local Store Buffer (a feature called *Store Forwarding*).
2. **To the Rest of the System (Other CPUs, DMA Engines, PCIe Accelerator Hardware):** They cannot see what is stuck inside your local CPU core's Store Buffer. They only see data once it has **drained** out into RAM/Cache.



**This is why memory barriers exist in drivers:**

A store barrier (like ARM's `dsb st` or Linux's `wmb()`) explicitly forces the CPU core to pause execution and **drain** its Store Buffer completely to memory *before* allowing the core to proceed to the next instruction (such as ringing a PCIe Doorbell register).