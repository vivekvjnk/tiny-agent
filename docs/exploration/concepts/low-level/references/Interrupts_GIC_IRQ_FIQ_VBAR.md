# FIQ vs IRQ
FIQ (Fast Interrupt Request) and IRQ (Interrupt Request) are two types of hardware interrupts used in computer processors, most notably in the Arm architecture. FIQ is built for fast, low-latency tasks, while IRQ handles normal, everyday system interrupts. [1, 2]  
Key Differences 

• Priority: FIQ has a higher priority than IRQ and can interrupt an active IRQ. An IRQ cannot interrupt an FIQ. 
• Speed: FIQ responds faster because it sits at the very end of the exception vector table, allowing the code to run directly without a jump instruction. IRQ requires a branch (jump) instruction to find its handler code. 
• Registers: FIQ has more dedicated banked (shadow) registers (`r8` to `r14` ). This means the CPU does not need to save many register states before running the code, cutting down delay. IRQ shares more registers and requires saving states. 
• Number of Sources: FIQ is meant for only one main, high-priority task (like a data transfer or secure monitor). IRQ handles multiple general devices like keyboards, timers, and USB ports. [2, 5, 6]  

[1] https://support.arm.com/documentation/den0013/0400/Exception-Handling/Exception-priorities/FIQ-and-IRQ
[2] https://en.wikipedia.org/wiki/Fast_interrupt_request
[3] https://en.eeworld.com.cn/news/mcu/eic617873.html
[4] https://developer.arm.com/documentation/dui0203/latest/handling-processor-exceptions/armv6-and-earlier--armv7-a-and-armv7-r-profiles/interrupt-handlers
[5] https://stackoverflow.com/questions/973933/what-is-the-difference-between-fiq-and-irq-interrupt-system
[6] https://stackoverflow.com/questions/973933/what-is-the-difference-between-fiq-and-irq-interrupt-system



# VBAR 

VBAR_EL3 (Vector Base Address Register EL3) holds the base address of the exception vector table for any exceptions taken to Exception Level 3 (EL3) in ARMv8/v9 AArch64 architecture. [1, 2] 

## Key Details
* Purpose: Points to where the processor jumps when an exception, interrupt, or synchronous abort targets EL3 (the highest privilege level, typically used for secure monitor or firmware code). [2, 3, 4] 
* Alignment Requirement: The address set in VBAR_EL3 must be properly aligned (typically 2KB or 128-byte aligned depending on the vector table layout restrictions, usually on a 2048-byte boundary for standard ARM architecture vector tables).
* Common Issues:
* Misalignment of the vector table address can trigger a fault or unexpected behavior.
   * Failing to set VBAR_EL3 before an exception occurs causes the processor to use an invalid or default reset vector, leading to crashes or unhandled exceptions.
   * Tagged address bits [55:52] must be zero, or it can cause a recursive exception. [5] 

[1] [https://www.wadixtech.com](https://www.wadixtech.com/blog/armv8-a-vector-table-exception-handling)
[2] [https://developer.arm.com](https://developer.arm.com/documentation/ddi0500/e/system-control/aarch64-register-descriptions/vector-base-address-register--el3?lang=en)
[3] [https://support.arm.com](https://support.arm.com/documentation/102412/0103/Privilege-and-Exception-levels/Exception-levels)
[4] [https://developer.arm.com](https://developer.arm.com/documentation/ddi0500/j/Programmers-Model/ARMv8-A-architecture-concepts/Exception-levels)
[5] [https://developer.arm.com](https://developer.arm.com/documentation/ddi0487/maa/-Part-D-The-AArch64-System-Level-Architecture/-Chapter-D24-AArch64-System-Register-Descriptions/-D24-2-General-system-control-registers/-D24-2-216-VBAR-EL3--Vector-Base-Address-Register--EL3-)



There are dedicated VBAR registers for EL1, EL2, and EL3 in the ARMv8/v9 AArch64 architecture.
Specifically, they are named VBAR_EL1, VBAR_EL2, and VBAR_EL3. Each exception level has its own independent register to point to its own distinct vector table.

## How Many Base Addresses Are Stored?
Exactly one base address is stored in each VBAR register.
The VBAR register does not hold the individual handlers for every type of interrupt or error. Instead, it holds a single pointer to the start (base address) of an entire vector table.
Inside that vector table, there are 16 distinct entry points (slots), arranged sequentially. Each slot corresponds to a specific type of exception and the state the processor was in when it happened. Each slot is exactly 128 bytes (32 instructions) long, which is enough room to either write a small, immediate handler or jump (B or BR) to a larger handler elsewhere in memory.

Total memory usage by one table : 128 * 16 = 2048 Bytes ~ 2KB

Considering 3 exception levels : 2KB * 3 = 6KB of RAM memory to store all the exception handlers
## Structure of the 16 Table Slots
The **table pointed** to by a single VBAR register is divided into four major groups, each containing four exception types:

| Source of Exception | Synchronous Aborts | IRQ (Interrupt) | FIQ (Fast Interrupt) | SError (System Error) |
|---|---|---|---|---|
| From current EL (using SP_EL0) | Slot 1 | Slot 2 | Slot 3 | Slot 4 |
| From current EL (using SP_ELx) | Slot 5 | Slot 6 | Slot 7 | Slot 8 |
| From lower EL (using AArch64) | Slot 9 | Slot 10 | Slot 11 | Slot 12 |
| From lower EL (using AArch32) | Slot 13 | Slot 14 | Slot 15 | Slot 16 |

## Where is the Storage For These?

* The VBAR Registers: These are dedicated, built-in hardware system registers located physically inside the ARM processor core's execution unit. They do not occupy space in your system RAM.
* The Actual Handlers: The vector table and the actual exception handling code are stored in system memory (RAM or ROM).

When an exception occurs, the hardware automatically looks at the internal VBAR register for the target Exception Level, calculates the correct offset for the slot (e.g., Base + 0x200 for an IRQ from a lower EL), and jumps to that address in RAM/ROM to run your code.


# VBAR and GIC(Generic Interrupt Controller)

To understand how VBAR tables work alongside GIC (Generic Interrupt Controller) tables, you have to look at them as a two-stage routing system.
The VBAR table belongs entirely to the CPU Core, while the GIC belongs to the System Interrupt Hardware. They are completely different in structure, but they work together sequentially to handle a peripheral interrupt. [1, 2] 

## 1. How are VBAR slots different from GIC tables?
The difference lies in their scope and function. VBAR acts as a broad "traffic cop" for the CPU, while the GIC table acts as a detailed "lookup directory" for peripherals. [2, 3] 

* VBAR Slots (CPU level): The CPU core has no idea which hardware device fired an interrupt (e.g., whether it was a timer, a keyboard, or a USB port). It only knows that a generic IRQ or FIQ has been asserted. The VBAR table's only job is to provide a single entry point for the CPU to jump into a generic interrupt handling routine. [2, 3, 4] 
* GIC Tables (Interrupt Controller level): The GIC is external to the CPU core. It physically hooks up to hundreds of distinct interrupt lines from all your hardware peripherals. It knows exactly which device fired (assigning each a unique INTID, like INTID 33 for a UART port). [2] 

## Direct Comparison

| Feature | VBAR Slots (Exception Vectors) | GIC / Software Interrupt Tables |
|---|---|---|
| Who owns it? | The ARM CPU Core itself. | The GIC hardware and the OS kernel. |
| Triggered by? | Any CPU exception (Page faults, Syscalls, IRQs, SErrors). | Specific hardware peripheral signals (Timers, PCIe, UART). |
| Table Layout | Strict, hardcoded 16-slot architectural layout. | Dynamic array of function pointers matching device INTIDs. |
| Content | Executable instructions (code blocks up to 128 bytes). | Memory addresses (pointers to C/C++ functions). |


## 2. How do they correlate? (The Sequence)
There is a direct correlation in execution flow. The VBAR IRQ slot acts as the gateway to the GIC software table. [2, 4] 
When a peripheral (like a keyboard) sends an interrupt, the hardware and software execute this exact chain: [5] 

   1. Peripheral fires $\rightarrow$ Signals the GIC. [2, 5] 
   2. GIC prioritizes it $\rightarrow$ Asserts the generic IRQ pin on the CPU Core. [2] 
   3. CPU core takes action $\rightarrow$ Looks at VBAR_EL1, automatically adds the math offset for an IRQ, and jumps straight into that VBAR slot (e.g., Slot 3 or Slot 7). [4, 6, 7] 
   4. VBAR Slot Code executes $\rightarrow$ This code saves your CPU registers. Then, it reads a specific GIC hardware system register (like ICC_IAR1_EL1) to ask: "Hey GIC, which device actually caused this?" [1, 2, 5, 6] 
   5. GIC responds $\rightarrow$ Returns the unique INTID (e.g., 33). [8] 
   6. Kernel handles the rest $\rightarrow$ The VBAR code takes that ID, uses it as an index into the GIC software array, and jumps to the specific C function (uart_interrupt_handler()). [5, 9] 


## 3. Does system memory store both?
Yes, system memory stores both, but they are kept in completely separate regions of RAM/ROM and structured differently. [6, 9] 
## 1. The VBAR Block (2KB Allocation)
Your firmware or operating system allocates exactly 2048 bytes (16 slots $\times$ 128 bytes) in memory for the CPU core. This block must be 2KB-aligned. Because 128 bytes can only fit 32 instructions, this space is purely used for "trampoline" code—it saves registers, queries the GIC, and immediately jumps out to the main OS code. [3, 6, 7] 
```S
/* Example of what is inside that 2KB RAM block */
.balign 2048
VBAR_TABLE_START:
    /* Slot 1 to 2: Synchronous handler entries... */
    ...
    .balign 128
    /* Slot 3: Generic IRQ Trampoline */
    sub     sp, sp, #256         // Allocate stack
    stp     x0, x1, [sp]         // Save registers
    bl      gic_hardware_parser  // Go parse the GIC and call OS handlers!
    ...
```
## 2. The GIC / OS Driver Array (Variable Allocation)
Elsewhere in your system RAM, the Operating System allocates a completely separate array of standard function pointers (often called the IRQ Descriptor Table). [9] 

* If a chip supports up to 256 unique interrupts, this table will be an array of 256 pointers ($256 \times 8 \text{ bytes} = 2\text{KB}$ of pointers).
* The actual, massive C functions containing the logic (like processing network packets or USB data) live dynamically throughout kernel memory space, completely unconstrained by VBAR's strict layout. [9, 10, 11] 

[1] [https://www.wasilzafar.com](https://www.wasilzafar.com/pages/series/arm-assembly/arm-assembly-11-exception-levels.html)
[2] [https://www.realdigital.org](https://www.realdigital.org/doc/87be521204ed2c4447d567b666961ce8)
[3] [https://developer.arm.com](https://developer.arm.com/documentation/102412/latest/Handling-exceptions/Taking-an-exception)
[4] [https://pyjamacafe.com](https://pyjamacafe.com/posts/arm64-day2-irq-handling/)
[5] [https://www.reddit.com](https://www.reddit.com/r/embedded/comments/m182sw/interrupt_vector_table_question/)
[6] [https://www.wadixtech.com](https://www.wadixtech.com/blog/armv8-a-vector-table-exception-handling)
[7] [https://dev.to](https://dev.to/ripan030/linux-kernel-interrupt-handling-part-2-fe1)
[8] [https://www.sciencedirect.com](https://www.sciencedirect.com/topics/engineering/interrupt-vector)
[9] [https://www.realdigital.org](https://www.realdigital.org/doc/87be521204ed2c4447d567b666961ce8)
[10] [https://onlinedocs.microchip.com](https://onlinedocs.microchip.com/oxy/GUID-4F4EDA17-350C-4EC3-B601-9F459DD028EF-en-US-4/GUID-273FB30C-2560-4BD4-8B0F-7459DA5D4FD2.html)
[11] [https://www.geeksforgeeks.org](https://www.geeksforgeeks.org/operating-systems/interrupts/)
[12] [https://stackoverflow.com](https://stackoverflow.com/questions/44991264/armv8-exception-vectors-and-handling)


> You mentioned in the step 6 of GIC - VBAR correlation that the "kernel handles the rest". Does that mean, the VBAR handler code may reside as part of the kernel? I'm aware GIC interrupt handlers can be part of kernel or some privileged plugins to the kernel. But what about VBAR?

Yes, absolutely. The VBAR vector table and its 16 slot handlers are compiled directly into, and reside inside, the operating system kernel (or the respective privileged firmware, like a Hypervisor or Secure Monitor).
In fact, the VBAR handlers are among the most critical, core components of the kernel itself.

## Where VBAR Resides in the System Architecture
Because the VBAR table handles core CPU exceptions like memory faults (Synchronous Aborts), system calls (SVC instructions), and hardware interrupts, it cannot safely live anywhere else.
Here is how it is split across the different privilege levels:

* OS Kernel (EL1): The kernel code (e.g., Linux, macOS, Windows, or an RTOS) allocates and contains the VBAR_EL1 table. When the kernel boots, one of its very first tasks is to write the memory address of this compiled table into the CPU's VBAR_EL1 register.
* Hypervisor (EL2): If you are running virtual machines (like KVM or Xen), the hypervisor code contains its own distinct vector table, which it points to using VBAR_EL2.
* Secure Monitor/Firmware (EL3): Low-level firmware (like ARM Trusted Firmware or your device’s bootloader) contains a third vector table, pointed to by VBAR_EL3.


## Why VBAR Must Be Part of the Core Kernel (and Not a Plugin)
While peripheral GIC device drivers (like a Wi-Fi or USB driver) can be dynamically loaded as kernel modules or plugins, the VBAR code cannot be a plugin for two major reasons:
## 1. It Handles Survival-Critical Faults
If a user program tries to access an invalid memory address, the CPU immediately generates a Data Abort exception and jumps to VBAR Slot 1 or 5.

* If this handler were inside a removable plugin and that plugin wasn't loaded, the CPU would have nowhere to jump.
* This would trigger a Double Fault / Fatal Exception, immediately freezing or resetting the physical processor. The core memory management unit (MMU) logic and VBAR must be built into the un-swappable, permanent base of the kernel.

## 2. The Chicken-and-Egg Problem
A dynamic kernel plugin or module relies on the kernel's memory management, scheduling, and interrupt infrastructure to load and run. However, the CPU cannot safely run a multitasking operating system without VBAR already active to handle timer ticks and system calls. Therefore, VBAR must exist before any plugins can even be loaded.

## Summary of the Code Lifecycle

1. At Compile Time: When the Linux or RTOS kernel is built, the 2KB VBAR assembly block is compiled right alongside the core kernel image (often placed in a section called .text.exceptions).
2. At Boot Time: The kernel image is loaded into RAM. The boot CPU executes early initialization assembly code, takes the RAM address of that compiled table, and runs:
```S
adr     x0, VBAR_TABLE_START  // Get RAM address of the compiled table
msr     vbar_el1, x0          // Commit it directly to the CPU hardware
```
3. At Runtime: The table remains locked in that specific RAM location for as long as the operating system is running.

