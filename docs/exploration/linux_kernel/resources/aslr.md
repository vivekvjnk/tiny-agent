# ASLR in Linux (High-Entropy Summary)

## Core Definition
* **ASLR:** Address Space Layout Randomization.
* **Mechanism:** Randomizes memory locations (stack, heap, libraries) at execution.
* **Objective:** Prevents predictable memory targeting to block binary exploits.

## Security Mechanics
* **Disruption:** Invalidates hardcoded memory addresses in exploit payloads.
* **Outcome:** Triggers process crashes (Segmentation Faults) instead of code execution.
* **Scope:** Covers user-space applications and system kernels via **KASLR** (Kernel ASLR).

## Operational States (`/proc/sys/kernel/randomize_va_space`)
* **`0` (Disabled):** Static, predictable memory layouts. High exploit vulnerability.
* **`1` (Conservative):** Randomizes stack, virtual dynamic shared objects (vDSO), and shared memory segments.
* **`2` (Full):** Randomizes Level 1 regions plus the memory heap. *Standard Linux default.*