## Technical Summary: Booting into ARM64 EL3h via QEMU## 🛑 The Root Problem
When launching QEMU with the -kernel flag, QEMU acts as an internal Linux bootloader. It performs platform initialization at EL3 and automatically drops execution to Non-Secure EL1/EL2 before hitting your -S GDB breakpoint.
Because the CPU context drops to EL1, the Secure Configuration Register (scr_el3) becomes completely hidden from GDB and the QEMU monitor due to architectural hardware security boundaries.
## 🛠️ The Solution: Custom FSBL Reset Hijack
To inspect or alter scr_el3, we treat QEMU's -bios interface as a First Stage Bootloader (FSBL). This hijacks the platform's absolute first execution cycle at the hardware reset vector before the automated Linux loader can downgrade privilege levels.
## 1. The Assembly Stub (boot.S)
Create a minimal assembly file that reads the targeted system register into a standard core register, triggers an inline breakpoint, and outlines a jump register path back to RAM space.
```S
.global _start
.section .text

_start:
    /* 1. Read hidden SCR_EL3, SPSR_EL3 and ELR_EL3 registers into standard registers X0, X1 and X2 */
    mrs x0, scr_el3
    mrs x1, spsr_el3
    mrs x2, elr_el3

    /* 2. Freeze the CPU right here while still executing in EL3h */
    brk #0

    /* 3. Hand control over to the Linux Kernel entry point in RAM */
    ldr x1, =0x40000000
    br  x1
```
## 2. Compilation & Flash Padding (The Fixes)
To ensure the binary executes flawlessly, it must be compiled with an absolute base address matching the physical flash reset vector (0x0). It must also be exactly padded to 64MB to satisfy QEMU's pflash0 block device validation constraints.
```bash
# Cross-compile targeted explicitly at physical memory address 0x0
aarch64-linux-gnu-gcc -nostdlib -nostartfiles -Wl,-Ttext=0x0 boot.S -o boot.elf
aarch64-linux-gnu-objcopy -O binary boot.elf boot.bin
# Pad perfectly to a 64MB Flash ROM Image capacity limit
dd if=boot.bin of=boot_padded.bin bs=1M count=64 conv=sync
```
## 3. Execution Command
Execute QEMU by supplying the custom bootloader to -bios while keeping your existing Linux target flags intact. This ensures QEMU pre-maps your Linux image, initrd, and command-line arguments into RAM right behind your running bootloader.

```bash
qemu-system-aarch64 \
  -M virt,secure=on \
  -cpu cortex-a57 \
  -m 512M \
  -bios boot_padded.bin \
  -kernel output/images/Image \
  -initrd output/images/rootfs.ext4 \
  -append "console=ttyAMA0 earlycon=pl011,0x9000000 rdinit=/init" \
  -nographic \
  -s -S
```
## 4. GDB/GEF Verification Process

   1. Connect via GEF: target remote :1234
   2. Advance execution past the first instruction block: si
   3. Print the hardware registers: monitor info registers

## Verified Output
```bash
PC=0000000000000004 X00=0000000000000030 ... PSTATE=400003cd -Z-- EL3h
* EL3h confirms successful execution inside the highest exception level.
* X00=0000000000000030 exposes the underlying hardware reset value of scr_el3.
```
===

# Verifying ELR_EL3 and SPSR_EL3
Then we went one step ahead to validate the output. We used a modified assembly FSB to monitor SPSR_EL3 and ELR_EL3. 
```S
.global _start
.section .text

_start:
       	/* 1. Read hidden SCR_EL3 register into readable register X0 */
        mrs x0, scr_el3
        mrs x1, spsr_el3
        mrs x2, elr_el3

        /* 2. trigger breakpoint here at EL3 */
        brk #0 
        /* 3. Jump straight to where QEMU loads kernel */
        ldr x1, =0x40008000
        br x1


```

```bash
(remote) gef➤  monitor info registers

CPU#0
PC=000000000000000c X00=0000000000000030 X01=0000000000000000
X02=0000000000000000 X03=0000000000000000 X04=0000000000000000
...
```

- x1 and x2 registers hold 0x0 because the `mathematically correct reset state` of these registers are 0x0
- CPU is in reset state with respect to these registers



## 🧩 Understanding SPSR_EL3 and ELR_EL3
Unlike scr_el3 (which holds physical hardware feature flags), SPSR and ELR are Exception Handling Registers: [2] 

   1. SPSR_EL3 (Saved Program Status Register): This register acts as a backup container. When an exception or interrupt forces the CPU to drop everything and jump into EL3, the hardware automatically snapshots the lower level's PSTATE and shoves it here so it can restore it later. [2, 3] 
   2. ELR_EL3 (Exception Link Register): This holds the memory return address. When an exception happens, the hardware logs the exact memory instruction address that caused the jump right here. [1] 

## 🕵️ Why are they reading as 0x0 right now?
Because we are performing a cold power-on boot via a custom bootloader, the CPU booted straight into EL3h out of reset. [3] 

* No exceptions or interrupts have taken place.
* Because no lower execution tier has ever run, and no context switch has ever occurred, the hardware backup slots have literally nothing to snapshot or save.
* Consequently, they retain their default cold-reset value: 0x0000000000000000. [1, 3] 


## 🧪 Proof of Execution: Write a Fake State
If you want to prove to yourself that your assembly code is reading the real hardware state registers correctly, we can inject a test value into them before reading them back.
Modify your boot.S file to look like this:
```S
.global _start
.section .text

_start:
    /* 1. Injected Test: Write unique mock values to SPSR and ELR */
    ldr x10, =0xDEADBEEF
    msr spsr_el3, x10
    
    ldr x11, =0xCAFEBABE
    msr elr_el3, x11

    /* 2. Read them straight back into your target inspection registers */
    mrs x0, scr_el3
    mrs x1, spsr_el3    /* Should now read back 0xDEADBEEF instead of 0x0 */
    mrs x2, elr_el3     /* Should now read back 0xCAFEBABE instead of 0x0 */
    
    /* 3. Freeze the CPU */
    brk #0

    /* 4. Hand control over to Linux Kernel */
    ldr x1, =0x40000000
    br  x1
```

- Using above code, we validated if spsr_el3 and elr_el3 registers are actually captured by x1 and x2. 

```bash
(remote) gef➤  monitor info registers

CPU#0
PC=0000000000000200 X00=0000000000000030 X01=00000000deadbeef
X02=00000000cafebabe X03=0000000000000000 
...
```

- But this lead us to another bug
- The FSB code triggered infinite loop behavior
  - Since we were capturing the logs in a file, this infinite loop created a 3.3 GB log file in the host system
- On further investigation, we found the exact reason behind this infinite loop



## 🕵️ Anatomy of the Infinite Loop
Look closely at the exception signature dumped in the log:

Taking exception 1 [Undefined Instruction] on CPU 0
...from EL3 to EL3
...with ELR 0x200
...to EL3 PC 0x200 PSTATE 0x3cd

This sequence reveals exactly why QEMU is spinning out of control:

   1. The Breakpoint Trap: The code executed through the instructions and hit brk #0.
   2. **The Missing Vector Table**: In ARMv8-A, executing a brk instruction triggers a synchronous exception. Because the CPU is currently in EL3, it looks for an **EL3 Exception Vector Table** (defined by the **vbar_el3** register) to figure out where the software handler for breakpoints lives.
   3. The Uninitialized Hardware Default: Because this is a raw cold-boot and **we haven't set up a vector table**, vbar_el3 defaults to a hardware fallback address of 0x0000000000000200.
   4. The Trap Loop: The CPU jumps to PC=0x200 to process the exception. However, there is no code written at 0x200—it contains uninitialized memory or garbage bytes. QEMU interprets these empty bytes as an Undefined Instruction, which triggers another exception. The processor tries to handle this new exception by jumping right back to the start of the vector table at PC=0x200.

This creates a high-speed, infinite exception loop (0x200 $\rightarrow$ Undefined Instruction $\rightarrow$ Jump to 0x200), causing QEMU to dump billions of trace lines a second until the system space fills up.

## 🛠️ The Fix: Spin Safely Without Tripping the Exception Engine
Avoid the brk trap entirely. Instead, use a classic infinite branch loop. This keeps the processor busy executing a single safe instruction forever.
Modify the boot_validator.S file to look like this:

```S
.global _start
.section .text

_start:
    /* 1. Injected Test: Write unique mock values to SPSR and ELR */
    ldr x10, =0xDEADBEEF
    msr spsr_el3, x10
    
    ldr x11, =0xCAFEBABE
    msr elr_el3, x11

    /* 2. Read them straight back into your target inspection registers */
    mrs x0, scr_el3
    mrs x1, spsr_el3
    mrs x2, elr_el3
    
    /* 3. Freeze the CPU Safely (Replaces 'brk #0') */
freeze:
    b freeze           // Branches back to its own address infinitely

    /* 4. Hand control over to Linux Kernel */
    ldr x1, =0x40000000
    br  x1
```
## How to Debug This Safely now
1. Clear out that massive log file first (rm qemu_trace.log).
2. Boot the new image. QEMU will stay responsive and trace files will remain tiny.
3. Connect GEF. If we type continue or let the code execute, the CPU will safely spin at the freeze: label. We can break in with Ctrl+C inside GEF at any time, and the registers will stay permanently pinned at their valid values!

