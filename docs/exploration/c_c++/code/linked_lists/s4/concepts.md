# **4. Hash Tables: Static Memory Lock-Free Open Addressing Table**

**Concept:** Open Addressing (Linear Probing) & Dynamic Hardware Identifiers

**Scenario:** Microcontrollers often map device IDs, I2C addresses, or CAN filter IDs to internal driver structures without using runtime dynamic heap memory (`malloc`/`free`).

**Problem:** Design a fixed-size, thread-safe Hash Table in C that maps `uint32_t key` (e.g., CAN Bus Message ID) to `uint32_t value` (e.g., callback function pointer address).

* Use linear probing for collision resolution.
* **Rules:**
* Must run in a fixed static memory footprint (no dynamic allocations).
* Implement atomic insertion (`insert_atomic(key, val)`) and lookup (`lookup(key)`).
* Handle key collisions, table full conditions, and lock-free atomic updates (using Atomic Compare-And-Swap / CAS) so multiple tasks or ISRs can insert key-value pairs concurrently without corruption.
===


The scenario describes a lock-free, zero-heap open-addressing hash table tailored for real-time and embedded systems (like ARM Cortex-M microcontrollers).

Here is a breakdown of the core computer science and embedded engineering concepts at play:

---

## 1. Static Memory (No Dynamic Allocation)

* **What it means:** The memory for the table array is declared at compile time (e.g., as a global array or `static` buffer) rather than allocated on the heap at runtime via `malloc()` or `free()`.
* **Why it matters:** Dynamic heap allocation causes **memory fragmentation**, introduces nondeterministic execution time (unacceptable in real-time systems), and carries the risk of running out of memory mid-operation. Using a fixed-size static array ensures predictable memory footprint and execution latency.

---

## 2. Open Addressing with Linear Probing

* **What it means:** Unlike separate chaining (which uses linked lists for each hash bucket), open addressing stores all key-value pairs directly inside the table array itself.
* **Linear Probing:** When inserting a key, you calculate its initial index via a hash function (`idx = hash(key) % capacity`). If `table[idx]` is already occupied by a different key, you sequentially check the next slot (`idx + 1`, `idx + 2`, ...) until an empty slot is found.
* **Trade-offs:**
* *Pros:* Cache-friendly and avoids pointers/node allocation.
* *Cons:* Requires monitoring the **Load Factor** (ratio of occupied slots to total capacity) to prevent clustering and keep search times near $O(1)$. When full, insertion returns an error (e.g., table full condition).



---

## 3. Lock-Free Atomic Concurrency (CAS)

* **The Problem:** In multi-tasking environments (FreeRTOS, ThreadX) or ISR (Interrupt Service Routine) contexts, two threads might try to insert into the exact same empty slot at the exact same moment, causing data corruption or lost updates.
* **The Lock-Free Solution:** Instead of using mutexes or disabling interrupts (which causes latency bottlenecks), lock-free algorithms rely on hardware-supported **Atomic Primitives**, specifically **Compare-And-Swap (CAS)**.
* **How CAS works:** `Atomic CAS` takes a memory location, an expected old value, and a new value:

$$\text{CAS}(\text{address}, \text{expected\_old\_value}, \text{new\_value})$$



The hardware updates `address` to `new_value` **if and only if** `address` currently equals `expected_old_value`. This operation is guaranteed to be uninterruptible by other CPU cores or interrupts.

---

## 4. How Atomic Insertion Works (Step-by-Step Logic)

To prevent race conditions without locks, key-value insertions follow a state transition strategy:

1. **Sentinel Values:** An empty slot key is designated using a reserve pattern (e.g., `KEY_EMPTY = 0` or `0xFFFFFFFF`).
2. **Atomic Key Claiming:**
* A thread finds an empty slot candidate (`slot->key == KEY_EMPTY`).
* It attempts to claim the slot via CAS:

$$\text{CAS}(\&slot\to key, \text{KEY\_EMPTY}, \text{target\_key})$$


* **If CAS succeeds:** This thread won the race. It now safely writes the corresponding value (`slot->val = val`) and updates the state (or marks the value valid).
* **If CAS fails:** Another task claimed the slot a fraction of a clock cycle earlier. The thread moves to the next linear probing index (`idx + 1`) and tries again.


3. **Atomic Value Synchronization:** Read/write ordering must be enforced using **Memory Barriers** or atomic stores (`atomic_store` with sequential consistency or acquire-release semantics) so readers looking up a key won't read an uninitialized or partially written value.

---

## Key-Value Map Representation in Hardware Registers

```
   Index        Slot Key             Slot Value
  +-------+--------------------+--------------------+
  |   0   | 0x00000000 (EMPTY) | 0x00000000         |
  +-------+--------------------+--------------------+
  |   1   | 0x00000412 (CAN 1) | 0x0800A410 (ISR 1) |  <-- Claimed via CAS
  +-------+--------------------+--------------------+
  |   2   | 0x00000413 (CAN 2) | 0x0800A520 (ISR 2) |  <-- Linear probe fallback
  +-------+--------------------+--------------------+
  |   3   | 0x00000000 (EMPTY) | 0x00000000         |
  +-------+--------------------+--------------------+

```