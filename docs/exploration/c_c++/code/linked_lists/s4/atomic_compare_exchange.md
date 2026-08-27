# atomic_compare_exchange
---
std::atomic::compare_exchange_strong is a fundamental synchronization primitive in C++ used to implement lock-free data structures. Commonly referred to as a Compare-And-Swap (CAS) operation, it atomically compares the value of an atomic variable with an expected value. If they are equal, it replaces the atomic variable's value with a desired value. [1, 2, 3] 
Unlike its "weak" counterpart, the strong version guarantees it will never fail spuriously; it only returns false if the values actually mismatch. [1, 4] 
------------------------------
## How It Works Under the Hood
The method evaluates the atomic variable and acts based on the result: [3] 

* If True (Match): Replaces the value in the atomic object with desired and returns true.
* If False (Mismatch): Leaves the atomic object untouched, copies the current actual value of the atomic object into expected, and returns false. [3] 

// Conceptual pseudo-code of what happens ATOMICALLY:
```cpp
bool compare_exchange_strong(T& expected, T desired) {
    if (this->value == expected) {
        this->value = desired;
        return true;
    } else {
        expected = this->value; // 'expected' is updated with the current value
        return false;
    }
}
```
------------------------------
## Code Example: Implementing a Lock-Free Stack Node
A classic use case for compare_exchange_strong is safely inserting a node into a lock-free singly linked list without using a mutex.
```cpp
#include <atomic>
struct Node {
    int data;
    Node* next;
    Node(int data) : data(data), next(nullptr) {}
};
class LockFreeStack {private:
    std::atomic<Node*> head{nullptr};
public:
    void push(int data) {
        Node* new_node = new Node(data);

        // Put the current head into 'expected'
        new_node->next = head.load();

        // Loop until head successfully points to new_node.
        // If another thread changed 'head' in the meantime, 
        // 'compare_exchange_strong' will fail, automatically update 
        // 'new_node->next' with the NEW head, and try again.
        while (!head.compare_exchange_strong(new_node->next, new_node)) {
            // Loop body can remain empty because new_node->next 
            // is updated automatically on failure.
        }
    }
};
```
------------------------------
## Direct Comparison: Strong vs. Weak
C++ provides both compare_exchange_strong and compare_exchange_weak. Choosing between them depends heavily on your architecture and loop design. [5, 6] 

| Feature | compare_exchange_strong | compare_exchange_weak |
|---|---|---|
| Spurious Failures | Not allowed. Only fails if values mismatch. | Allowed. Can return false even if values match. |
| Hardware Mapping | Maps to a loop of LL/SC instructions on ARM, or a single LOCK CMPXCHG on x86. | Maps directly to a single LL/SC instruction pair on ARM architectures. |
| Performance | Slower in a tight loop on ARM/PowerPC due to internal hardware verification. | Faster in a loop on ARM architectures. |
| Best Used When... | You want to check the condition once without an outer loop. | You are already using a loop that can tolerate occasional false retries. |

------------------------------
## Important Gotchas ⚠️

   1. Bitwise vs. Logical Equality: compare_exchange functions perform a bitwise comparison (std::memcpy style) rather than using operator==. If your data structure has padding bits (common in structures) or contains floating-point values like NaN, the comparison might fail even if the values are logically equal. [3, 7] 
   2. Pass-by-Reference Side Effect: The expected parameter is taken by reference. Remember that if the CAS operation fails, your expected variable will be overwritten with the thread's current atomic value. [3, 8] 


[1] [https://ryonaldteofilo.medium.com](https://ryonaldteofilo.medium.com/atomics-in-c-compare-and-swap-and-memory-order-part-2-64e127847e00)
[2] [https://algomaster.io](https://algomaster.io/learn/concurrency-interview/cpp-std-atomic-memory-orders)
[3] [https://cplusplus.com](https://cplusplus.com/reference/atomic/atomic/compare_exchange_strong/)
[4] [https://learn.microsoft.com](https://learn.microsoft.com/en-us/cpp/standard-library/atomic-functions?view=msvc-170)
[5] [https://www.qnx.com](https://www.qnx.com/developers/docs/8.0/com.qnx.doc.neutrino.lib_ref/topic/a/atomic_compare_exchange_strong.html)
[6] [https://kurzy.kpi.fei.tuke.sk](https://kurzy.kpi.fei.tuke.sk/c-reference/en/cpp/atomic/atomic/compare_exchange.html)
[7] [https://mooshak.dcc.fc.up.pt](https://mooshak.dcc.fc.up.pt/~oni-judge/doc/cppreference/reference/en/cpp/atomic/atomic_compare_exchange.html)
[8] [https://pubs.opengroup.org](https://pubs.opengroup.org/onlinepubs/9799919799/functions/atomic_compare_exchange_strong.html)


===

# Strong vs Weak

The fundamental difference is that compare_exchange_weak is allowed to fail spuriously (returning false even if the current value matches the expected value), while compare_exchange_strong guarantees a failure only if the values actually differ.
Because the weak version can occasionally fail for hardware-related reasons, it must almost always be used inside a loop.
------------------------------
## Why Does the "Weak" Version Exist?
The distinction comes down to performance and how different computer processors (CPUs) handle atomic operations.

* On x86 (Intel/AMD): The CPU natively supports a single, strong instruction called CMPXCHG. On this architecture, strong and weak perform identically under the hood.
* On ARM / PowerPC (Apple Silicon, Mobile devices): The CPU doesn't have a single "compare and swap" instruction. Instead, it uses a pair of instructions: LL (Load-Linked) and SC (Store-Conditional).

On ARM, if an interrupt, context switch, or a simple memory cache line migration happens between the LL and the SC instructions, the SC instruction fails.

* If you use compare_exchange_weak, C++ maps it directly to this LL/SC pair. If a hardware glitch happens, it just returns false.
* If you use compare_exchange_strong, C++ is forced to wrap that LL/SC pair inside an internal hardware loop to catch those accidental glitches and retry until it's absolutely sure it failed due to a real value mismatch.

------------------------------
## Side-by-Side Comparison

| Feature | compare_exchange_strong | compare_exchange_weak |
|---|---|---|
| Spurious Failures | Forbidden. Fails only if actual != expected. | Allowed. Can fail randomly even if actual == expected. |
| Performance (ARM) | Slightly slower (due to internal hardware safety loops). | Faster (maps perfectly to 1-to-1 CPU instructions). |
| Performance (x86) | Identical to weak. | Identical to strong. |
| Loop Requirement | Optional. Used when the algorithm logic demands a retry. | Mandatory. You must loop to catch spurious failures. |

------------------------------
## Code Comparison: When to Use Which## 1. Use WEAK when you already need a loop anyway
If your algorithm naturally requires a loop to retry until success (like our lock-free stack push from earlier), always prefer weak.
Since you already have a while loop to handle multiple threads colliding, that same loop will effortlessly catch any "spurious" hardware failures without adding extra overhead.

// PREFERRED: Weak inside a loop// If a spurious failure happens, the while loop just tries again. No harm done.while (!head.compare_exchange_weak(new_node->next, new_node)) {
    // Empty loop body
}

## 2. Use STRONG when you are NOT using a loop
If you are doing a one-off check where you only want to attempt a swap exactly once, use strong. If you used weak here, your code might fail and exit simply because the CPU blinked, not because the data was actually changed by another thread.

// PREFERRED: Strong for single-shot attempts// We only want to try once. If it fails, we want to know it failed for a REAL reason.bool flag_was_raised = flag.compare_exchange_strong(expected_state, desired_state);if (flag_was_raised) {
    do_something_once();
}

