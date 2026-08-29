## Summary of Key Points


* A32 vs. T32 (ARM vs. Thumb): These are modern names for the legacy 32-bit instruction sets. A32 (ARM) uses fixed 32-bit instructions optimized for raw performance and widespread conditional execution. T32 (Thumb) uses mixed 16/32-bit instructions to optimize code density, achieving a much smaller memory footprint.

* 64-Bit Execution (AArch64): In 64-bit mode (as seen in your QEMU Cortex-A53 dump), 32-bit concepts like the T bit and CPSR do not exist. Instead, the processor state is governed by PSTATE and 64-bit registers (X0–X30), operating at distinct Exception Levels (like EL1h).

* The AArch64 to AArch32 Transition: A 64-bit core can transition to a 32-bit state only when dropping to a lower Exception Level (typically a 64-bit kernel at EL1 dropping to a 32-bit application at EL0) via an Exception Return (ERET) instruction.

* PSTATE and CPSR Dynamics During Transition:

* Preparation: While in AArch64 EL1, the kernel writes to the Saved Program Status Register (SPSR_EL1). It forces Bit 4 (nRW) to 1 (targeting 32-bit state), sets Bits 3:0 to 0000 (User mode), and sets Bit 5 (T) to determine if the app starts in A32 (0) or T32 (1) mode.
   * The Jump: Upon executing ERET, the hardware instantly copies SPSR_EL1 into the active internal PSTATE, causing the hardware execution pipeline to physically reconfigure from 64-bit decoding to 32-bit decoding.
   * The Transformation: Once at EL0, the internal PSTATE formatting disappears and is projected as a traditional AArch32 CPSR register. The core condition flags (N, Z, C, V) pass through directly, while Bit 5 actively materializes as the operational T bit to govern the 32-bit instruction decoding state.


* The .balign Directive: This is a compile-time assembler directive, not a runtime CPU instruction. It inserts padding bytes (0x00 or NOPs) to align code/data to power-of-2 boundaries. It consumes zero runtime CPU cycles and performs zero load/store operations, but it drastically speeds up actual LDR/STR hardware performance by preventing unaligned memory access penalties.
