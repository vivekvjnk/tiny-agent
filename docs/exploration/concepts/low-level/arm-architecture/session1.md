This execution plan for **Session 1** focuses on mastering ARM64 architectural transitions, early exception context, and kernel handoff invariants.

---

# Phase 1: Environment Spin-Up & Base State Verification

Run QEMU with a frozen CPU state to attach GDB and verify the CPU reset vector and architectural state before executing any instructions.

## **1. Launch QEMU Target Terminal**

```bash
qemu-system-aarch64 \
  -M virt,secure=on \
  -cpu cortex-a57 \
  -m 512M \
  -kernel output/images/Image \
  -initrd output/images/rootfs.ext4 \
  -append "console=ttyAMA0 earlycon=pl011,0x9000000 rdinit=/init" \
  -nographic \
  -s -S

```

Concise breakdown of the important parameters passed the QEMU command:

| Parameter | Purpose | What it Does in Your Command |
|---|---|---|
| -M | Machine platform | Selects the generic virt board and turns on ARM TrustZone (secure=on). |
| -cpu | Processor model | Emulates a 64-bit ARM Cortex-A57 core. |
| -m | System memory | Allocates 512 MB of RAM to the virtual machine. |
| -kernel | Linux kernel image | Boots the uncompressed Linux kernel binary (Image). |
| -initrd | Initial RAM filesystem | Loads the root filesystem image into RAM before booting. |
| -append | Kernel boot arguments | Configures the runtime environment with three critical instructions: <br> • earlycon=pl011,0x9000000: Bypasses normal drivers to print boot logs immediately. It hardcodes the PL011 UART hardware driver at the specific base memory address 0x09000000 (the standard serial port for QEMU's virt machine) so you can catch early boot crashes. <br>• console=ttyAMA0: Hands over output to the standard ARM serial driver once the kernel is fully loaded. <br>• rdinit=/init: Launches /init as the very first user-space program inside your ramdisk. |
| -nographic | Display mode | Disables GUI windows and routes the VM console directly to your terminal. |
| -s | GDB debugging | Opens a GDB debugger server on backend TCP port 1234. |
| -S | Freeze on startup | Pauses the CPU at power-on so you can connect GDB before the kernel boots. |
| -gdb tcp::<port>| Setup GDB with protocol::port| Allows to specify any free TCP port on your host system.|




Once the above command is ran, the terminal window will get stuck with no output. QEMU is waiting at the base memory address 0x09000000 for gdb server to send instructions. Now open a new terminal and attach GDB client to the QEMU gdb server.


## **2. Attach GDB with GEF in a Second Terminal**

```bash
gdb -ex "target remote :1234"
```



Breakdown of the command used to connect a GDB client to the paused QEMU virtual machine:

| Component | Purpose | What it Does in Your Command |
|---|---|---|
| gdb | The Debugger Client | Launches the GNU Debugger application on your host machine. |
| -ex | Execute Command | Tells GDB to immediately run the following string as an internal debugger command right after it starts up, saving you from typing it manually. |
| "target remote ..." | Remote Connection | The specific GDB command that tells the debugger to switch from looking at local host processes to looking at an external, remote target system. |
| :1234 | Target Address / Port | Specifies the location of the remote target. Leaving the space before the colon blank defaults to localhost (your own computer), and 1234 is the exact network port where QEMU's backend debugger server is listening. |


Expected output:

```bash
[ Legend: Modified register | Code | Heap | Stack | String ]
───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────── registers ────
[!] Command 'registers' failed to execute properly, reason: max() iterable argument is empty
───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────── stack ────
[!] Command 'dereference' failed to execute properly, reason: Failed to determine memory layout
───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────── code:generic: ────
[!] Command 'context' failed to execute properly, reason: 
Python Exception <class 'RuntimeError'>: No session started
❌️ Error occurred in Python: No session started
(remote) gef➤  
```
GEF is explicitly designed for debugging standard user-space Linux applications (like CTF binaries or standard user programs) rather than bare-metal firmware, bootloaders, or raw Linux kernels. Because your QEMU virtual machine is paused at the absolute first instruction of the CPU (before virtual memory layout or the operating system exists), GEF's automation scripts completely break down.

## **3. Initial Register Snapshot Verification**
Execute the following inside GDB to confirm initial execution mode:

```bash
(remote) gef➤  monitor info registers

CPU#0
 PC=0000000040000000 X00=0000000000000000 X01=0000000000000000
X02=0000000000000000 X03=0000000000000000 X04=0000000000000000
X05=0000000000000000 X06=0000000000000000 X07=0000000000000000
X08=0000000000000000 X09=0000000000000000 X10=0000000000000000
X11=0000000000000000 X12=0000000000000000 X13=0000000000000000
X14=0000000000000000 X15=0000000000000000 X16=0000000000000000
X17=0000000000000000 X18=0000000000000000 X19=0000000000000000
X20=0000000000000000 X21=0000000000000000 X22=0000000000000000
X23=0000000000000000 X24=0000000000000000 X25=0000000000000000
X26=0000000000000000 X27=0000000000000000 X28=0000000000000000
X29=0000000000000000 X30=0000000000000000  SP=0000000000000000
PSTATE=400003c5 -Z-- NS EL1h    FPU disabled
```

*Expected Verification*: Confirm `PC` is pointing to physical boot RAM (`0x40000000` or `0x0` depending on TF-A/ROM loading), and evaluate whether `PSTATE.EL` is running in EL3 or EL1.

### Observations
Since we used monitor info registers, QEMU pulled the real-time internal CPU execution state and decoded it for us. Here is exactly what those specific flags (-Z-- NS EL1h) mean right now at boot:

| Component | Value/Flag | What it tells us about the CPU state |
|---|---|---|
| PC | 0x40000000 | The Program Counter is at the very start of RAM memory for the QEMU virt machine board. |
| -Z-- | Condition Flags | The Zero flag (Z) is active, while the Negative (N), Carry (C), and Overflow (V) flags are cleared. |
| NS | Non-Secure | The CPU is executing code in Non-Secure world. (Note: Even though we passed secure=on in QEMU, the virtual bootloader drops down to Non-Secure space before handing off execution to a standard Linux kernel). |
| EL1h | Exception Level | The CPU is running in Exception Level 1 (Kernel/OS mode) utilizing the dedicated SP_EL1 stack pointer (h stands for "handler" or using the target EL's stack pointer). |

Because the CPU is paused at the absolute first instruction of physical execution, the generic registers (X00 through X30) and the Stack Pointer (SP) are logically entirely zeroed out, awaiting initialization by the kernel setup routine.


### How to View the PSTATE Data in GDB
```bash
(remote) gef➤  info registers pstate pc sp
❌️ Invalid register `pstate'
```

- Above command failed because PSTATE is **not** a hardware register in ARMv8-A. It is rather a **Conceptual register**
- QEMU engine uses generic label `PSTATE=400003c5` to summarize its monitor output
- State is composed of multiple independent special purpose registers
  - CPSR : Current Program Status Register
  - SPSR : Saved Program Status Register
- Right command to see the PSTATE is 
  - `info registers pc sp cpsr`

To see the exact bits or value of what QEMU printed as PSTATE, we must query the cpsr register alias.

```
(remote) gef➤  info registers pc sp cpsr
pc             0x40000000          0x40000000
sp             0x0                 0x0
cpsr           0x400003c5          [ SP EL=1 F I A D BTYPE=0 Z ]
```


Explanation of the register values:

Here is a brief table explaining the registers and flags returned by your GDB command:

| Register | Hex Value | Decoded Flags / Meaning |
|---|---|---|
| pc | 0x40000000 | Program Counter: Points to the memory address of the very first instruction the CPU will execute. On the QEMU virt machine, this is the exact start of physical RAM. |
| sp | 0x0 | Stack Pointer: Uninitialized. The system has no stack memory environment configured yet, as the boot code hasn't set one up. |
| cpsr | 0x400003c5 | Current Program Status Register: Holds the CPU's real-time state, decoded by GDB as follows: <br>• Z: Zero Flag is set (meaning the result of the last internal comparison was zero). <br>• EL=1: The CPU is currently executing in Exception Level 1 (Kernel / Operating System mode). <br>• SP: The system is using the dedicated stack pointer for this exception level (SP_EL1). <br>• F, I, A, D: Interrupt Mask Bits (FIQ, IRQ, SError, and Debug exceptions) are all masked/disabled so the kernel can complete early initialization without interruption. <br>• BTYPE=0: [Branch Target Identification](../references/Branch_target_identification.md) is disabled or unconfigured. |


---

# Phase 2: Dissecting Exception Level Drops (`EL3` $\rightarrow$ `EL2` $\rightarrow$ `EL1`)

In ARM64 bare-metal firmware, lower exception levels cannot be "called" directly; transitions down occur exclusively via an `ERET` (Exception Return) instruction.

## **1. Architectural Controls Required for `ERET`**

To drop from Secure EL3 to Non-Secure EL2/EL1, firmware must program three specific control registers prior to executing `ERET`:

| Register | Target Field / Bit | Low-Level Value & Purpose |
| --- | --- | --- |
| **`SCR_EL3`** | Bit 0 (`NS`) | Set to `1` for Non-Secure World; `0` keeps target in Secure World.|
| **`SCR_EL3`** | Bit 10 (`RW`) | Set to `1` to enforce AArch64 execution state for lower ELs.|
| **`SPSR_EL3`** | Bits [3:0] (`M`) | Target Execution Mode: `0b1001` (EL2h) or `0b0101` (EL1h).|
| **`SPSR_EL3`** | Bits [9:6] (`DAIF`) | Set to `0b1111` (`0xF`) to mask Debug, SError, IRQ, and FIQ during switch.|
| **`ELR_EL3`** | Bits [63:0] | Physical/Virtual target execution address where execution resumes post-`ERET`.|

## **2. GDB Step-Through Exercise**
Set breakpoints on hardware exception returns or step through early assembly to watch system registers mutate:

```bash
# Inspect Secure Configuration Register
gef➤ info registers scr_el3

# Inspect Saved Program Status Register
gef➤ info registers spsr_el3

# Inspect Exception Link Register (Return Target Address)
gef➤ info registers elr_el3

```
Above commands failed to execute since we are at Exception Level 1. we are not authorized to see registers from higher exception level by executing commands from lower exception level.
```bash
(remote) gef➤  info registers scr_el3
❌️ Invalid register `scr_el3'
```

Issue is identified in the qemu VM bring up command.
```bash
qemu-system-aarch64   -M virt,secure=on \
    -cpu cortex-a57 \
    -m 512M \
    -kernel output/images/Image \
    -initrd output/images/rootfs.ext4 \
    -append "console=ttyAMA0 earlycon=pl011,0x9000000 rdinit=/init" \
    -nographic   -s -S
```
This command brings up the VM and uses the default boot loader from QEMU to drop exception levels. `-kernel` parameter is instructing QEMU is use the existing bootloader.

Instead we should use raw `-device loader` utility from QEMU

```bash
qemu-system-aarch64 \
  -M virt,secure=on \
  -cpu cortex-a57 \
  -m 512M \
  -device loader,file=output/images/Image,addr=0x40000000,cpu-num=0 \
  -initrd output/images/rootfs.ext4 \
  -append "console=ttyAMA0 earlycon=pl011,0x9000000 rdinit=/init" \
  -nographic \
  -s -S
```
This approach also didn't work out since `-device loader` utility doesn't support `-append` option. `-append` option require `-kernel` option. 
So we decided to follow a completely different approach. Create a custom `first stage bootloader` with a break point.
[Here's a detailed breakdown of the approach](../references/custom_fsb.md)

Final QEMU command becomes following:
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
  -s -S \
  -d cpu,int -D qemu_trace.log
```
---

# Phase 3: Validating the Kernel Handoff Invariant (`head.S`)

When the final bootloader phase (BL33/U-Boot) transfers execution to the Linux Kernel (`head.S`), hardware state must strictly adhere to the ARM64 boot specification.

**1. Execute Until Kernel Entry**
Set a breakpoint directly at the physical memory loading address of the kernel image:

```text
gef➤ b *0x40080000
gef➤ c
```
- Before executing above step, we should provide symbols file to the GDB
```bash
gef> file output/build/linux-6.18.7/vmlinux
```
- Now gdb will be able to map symbols to memory addresses
- First add break point at `head.S` 
- How to find `head.S` address
```bash
(remote) gef➤ si

(remote) gef➤  x/gx 0x40000020

(remote) gef➤ hbreak *0x40200000
(remote) gef➤ c
   0x401ffff4                  udf    #0
   0x401ffff8                  udf    #0
   0x401ffffc                  udf    #0
●→ 0x40200000                  ccmp   x18,  #0x0,  #0xd,  pl	// pl = nfrst
   0x40200004                  b      0x40ca20b8
   0x40200008                  udf    #0
   0x4020000c                  udf    #0
   0x40200010                  .inst  0x00d40000 ; undefined
   0x40200014                  udf    #0
─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────── threads ────
[#0] Id 1, stopped 0x40200000 in ?? (), reason: BREAKPOINT
───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────── trace ────
[#0] 0x40200000 → ccmp x18,  #0x0,  #0xd,  pl	// pl = nfrst
────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
(remote) gef➤  
```



**2. Execute Handoff Contract Assertion Checklist**

*Verification Evaluation Matrix*:
* **$x0$**: Must contain a valid physical DRAM address pointing to the Device Tree magic header `0xD00DFEED`.

```bash
# 1. Verify DTB Address Pointer in X0
(remote) gef➤  x/4wx $x0
0x48000000:	0xedfe0dd0	0x00001000	0x40000000	0x40200000

```


# 2. Verify Reserved Registers X1, X2, X3 are Zeroed
* **$x1, x2, x3$**: Must equal `0x0000000000000000`.

```bash
(remote) gef➤  print/x $x1
$1 = 0x0
(remote) gef➤  print/x $x2
$2 = 0x0
(remote) gef➤  print/x $x3
$3 = 0x0
(remote) gef➤ 
```

* **`SCTLR_EL1.M` (Bit 0)**: Must be `0` (MMU Off).
* **`SCTLR_EL1.C` (Bit 2)**: Must be `0` (Data Cache Off).

```bash
# 3. Verify MMU and D-Cache are Disabled
(remote) gef➤  info registers SCTLR
SCTLR          0xc50838            0xc50838
# 4. Verify Interrupt Masking Status
(remote) gef➤  monitor info registers
```

* **`PSTATE.DAIF` (Bits [9:6])**: Must be `0xF` (All asynchronous interrupts masked).

```bash
CPU#0
 PC=0000000040200000 X00=0000000048000000 X01=0000000000000000
X02=0000000000000000 X03=0000000000000000 X04=0000000040200000
X05=0000000000000000 X06=0000000000000000 X07=0000000000000000
X08=0000000000000000 X09=0000000000000000 X10=0000000000000000
X11=0000000000000000 X12=0000000000000000 X13=0000000000000000
X14=0000000000000000 X15=0000000000000000 X16=0000000000000000
X17=0000000000000000 X18=0000000000000000 X19=0000000000000000
X20=0000000000000000 X21=0000000000000000 X22=0000000000000000
X23=0000000000000000 X24=0000000000000000 X25=0000000000000000
X26=0000000000000000 X27=0000000000000000 X28=0000000000000000
X29=0000000000000000 X30=0000000000000000  SP=0000000000000000
PSTATE=400003c5 -Z-- NS EL1h    FPU disabled
```

```text
Hex 0xC5 = Binary 1 1 0 0  0 1 0 1
                  │ │ │ │  │ │ │ └── Bit 0
                  │ │ │ │  └───┴──── Bits 5:1 (Reserved/Attributes)
                  │ │ │ └─────────── Bit 6:  F (FIQ Mask)
                  │ │ └───────────── Bit 7:  I (IRQ Mask)
                  │ └─────────────── Bit 8:  A (SError Mask)
                  └───────────────── Bit 9:  D (Debug Mask)
```
1100 : IRQs and FIQs are not masked
- QEMU's primary loader is very minimal. It doesn't mask DAIF bits in PSTATE 
- Kernel masks DAIF as a failsafe mechanism
- QEMU temporarily halts all hardware peripheral timers and interrupt generation mechanisms during break point. Hence they would not trigger IRQ or FIQ during debuggin in this stage 



## Break at `start_kernel`

```bash
gef> b start_kernel
gef> c
   0xffff800080ab07b8 <thread_stack_cache_init+0000> ret    
   0xffff800080ab07bc <poking_init+0000> ret    
   0xffff800080ab07c0 <trap_init+0000> ret    
●→ 0xffff800080ab07c4 <start_kernel+0000> paciasp 
   0xffff800080ab07c8 <start_kernel+0004> stp    x29,  x30,  [sp,  #-112]!
   0xffff800080ab07cc <start_kernel+0008> adrp   x0,  0xffff800080be2000 <inet6_protos+392>
   0xffff800080ab07d0 <start_kernel+000c> add    x0,  x0,  #0xb80
   0xffff800080ab07d4 <start_kernel+0010> mov    x29,  sp
   0xffff800080ab07d8 <start_kernel+0014> stp    x19,  x20,  [sp,  #16]
─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────── threads ────
[#0] Id 1, stopped 0xffff800080ab07c4 in start_kernel (), reason: BREAKPOINT
───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────── trace ────
[#0] 0xffff800080ab07c4 → start_kernel()
[#1] 0xffff800080ab7678 → __primary_switched()
```

- Attempted to read x0 to see the DTB pointer
```bash
(remote) gef➤  x/4wx $x0
```
- This is wrong. Reading $x0 failed because register $x0 was clobbered when transitioning from early assembly (head.S) into C code (start_kernel). Per the ARM64 C Procedure Call Standard (AAPCS64), $x0 is used to pass function parameters and return values, so its value at kernel entry (0x48000000) was overwritten with 0xe11 during early C setup.
- In head.S, early kernel assembly copies the physical DTB pointer from $x0 into callee-saved register x21 before calling start_kernel. x21 = 0x48000000
```bash 
(remote) gef➤  info registers
...
x20            0xe11               0xe11
x21            0x48000000          0x48000000
x22            0x0                 0x0
...
(remote) gef➤  monitor xp /4wx 0x48000000
0000000048000000: 0xedfe0dd0 0x00001000 0x40000000 0x40200000
```

---

# Phase 4: Maximum Differential Knowledge Capture Sheet

* **The Synchronous Data Abort Trap on MMU Enabling**: Before setting `SCTLR_EL1.M = 1` in `head.S`, code memory must be **identity-mapped** ($VA = PA$). If virtual addresses do not match physical addresses the moment bit 0 is toggled, the CPU fetches the next instruction pipeline step from an unmapped VA, causing an immediate, fatal Prefetch Abort before `VBAR_EL1` can process it.

* **Alignment Fault Traps (`SCTLR_EL1.A`)**: When `SCTLR_EL1.A` (Bit 1) is set, any unaligned memory load/store (e.g., reading a 64-bit word from a non-8-byte aligned physical boundary) triggers an immediate alignment fault exception to EL1.

* **`PSTATE` Execution Modes (`SP_ELx` vs `SP_EL0`)**: The 'h' or 't' suffix on execution modes (e.g., `EL1h` vs `EL1t`) indicates which Stack Pointer register is active. `EL1h` means the handler is running in EL1 using `SP_EL1`; `EL1t` means it is executing in EL1 but using the thread/user stack pointer `SP_EL0`.

* **TLB Invalidation Invariants**: Before enabling the MMU, firmware must issue `tlbi vmalle1` (Translation Lookaside Buffer Invalidate by VMID) followed by a `dsb nsh` (Data Synchronization Barrier) and `isb` (Instruction Synchronization Barrier) to guarantee stale prefetch pipelines or reset TLB garbage entries are completely cleared.




# Key takeaways 
1. VBAR_EL3 exception
  - Uninitialized VBAR_EL3 exception force CPU to 0x0200
  - Since no exception handling was initialized here, it lead to another "Uninitialized VBAR_EL3" exception, essentially triggering an infinite loop.