[Reference](https://stackoverflow.com/questions/30190050/what-is-the-base-address-register-bar-in-pcie/44716618#44716618)
---
# Concept of Base Address Register(BAR) in PCIe
Each BAR is a small 32-bit memory address region that points to another (usually much larger) memory address region which I'll call the corresponding "BAR region". Each BAR tells the CPU the base address + width + other properties of its BAR region. The CPU can then read and write to that BAR region to talk to the PCIe device.

When you read or write to offsets within the BAR region, [TLP packets](https://xillybus.com/tutorials/pci-express-tlp-pcie-primer-tutorial-guide-1), the basic units of PCIe communication, are sent back and forth between the CPU and the PCIe device, which tells the PCIe device to do something or send something back.

Such reads and writes are the main way in which drivers interact with PCIe devices.

From the CPU's perspective, reads and writes to the BAR and it's region use the same instructions as regular RAM writes. However, they don't actually reach RAM. The [firmware](https://en.wikipedia.org/wiki/Firmware) configures the memory controller during PCIe enumeration to know about the BAR, and the memory controller sends them directly to PCIe without the need to go through RAM. The technique of using the same encodings for regular memory reads and writes and device control is called "[Memory-mapped I/O](https://en.wikipedia.org/wiki/Memory-mapped_I/O_and_port-mapped_I/O)" (MMIO), and reduces the need for specialized CPU instructions, at the cost of reducing the addressable RAM space a bit, which is not a big deal in 64-bit computers as we don't have enough RAM to fill 64-bits (16 billion GBs).

What reads and writes to specific addresses of a BAR region mean is defined by each specific PCIe device and completely device dependent, but typically:

* reads return status information such as:
  * what the device is currently doing, or how much work it has done so far
  * how the device has been configured
* writes:
  * configure how the device should operate

  * tell the device to start doing some work, e.g. write to disk, render a frame on the GPU, or send a packet over the network.

    A very common pattern in which such operations happen is:

    1. CPU writes input to RAM, e.g. coordinates of triangles for GPU to render, or HTTP request data to send over the network
    2. CPU tells the device where the input data is in RAM, and possibly where the output should go to e.g. another RAM address, some special memory like Video memory for GPU rendering, which network card Ethernet port for a network card
    3. device reads input data from main memory via DMA. Again, more TLP packets.
    4. device does some work
    5. device writes output data back to main memory via DMA
    6. device sends an interrupt to tell the CPU it finished its work
    7. CPU can now read any output from RAM, and possibly decide to schedule more work

Here's how the configuration spaces and BAR regions might look like on physical memory of a hypothetical device having:

- 2 functions
- 2 active BARs each, pointing to their corresponding regions

We'll explain "functions" on the section below.

```
| Virtual Address
| 
|  +--------------+
|  | Func 64:00.0 |
|  | Conf. Space  |
|  +--------------+
|  | BAR 0        |>--------------+
|  +--------------+               |
|  | BAR 1        |>----------+   |
|  +--------------+           |   |
|  | BAR 2        |           |   |
|  +--------------+           |   |
|  | BAR 3        |           |   |
|  +--------------+           |   |
|  | BAR 4        |           |   |
|  +--------------+           |   |
|  | BAR 5        |           |   |
|  +--------------+           |   |
|                             |   |
|  +--------------+           |   |
|  | Func 64:00.1 |           |   |
|  | Conf. Space  |           |   |
|  +--------------+           |   |
|  | BAR 0        |>------+   |   |
|  +--------------+       |   |   |
|  | BAR 1        |       |   |   |
|  +--------------+       |   |   |
|  | BAR 2        |>--+   |   |   |
|  +--------------+   |   |   |   |
|  | BAR 3        |   |   |   |   |
|  +--------------+   |   |   |   |
|  | BAR 4        |   |   |   |   |
|  +--------------+   |   |   |   |
|  | BAR 5        |   |   |   |   |
|  +--------------+   |   |   |   |
|                     |   |   |   |
|                     |   |   |   |
|                     |   |   |   |
|  +--------------+<--|---|---|---+
|  | Region of    |   |   |   |
|  | 64:00.0 BAR0 |   |   |   |
|  | (1 MiB)      |   |   |   |
|  +--------------+   |   |   |
|                     |   |   |
|  +--------------+<--+   |   |
|  | Region of    |       |   |
|  | 64:00.1 BAR2 |       |   |
|  | (2 MiB)      |       |   |
|  +--------------+       |   |
|                         |   |
|  +--------------+<------|---+
|  | Region of    |       |
|  | 64:00.0 BAR1 |       |
|  | (512 KiB)    |       |
|  +--------------+       |
|                         |
|  +--------------+<------+
|  | Region of    |
|  | 64:00.1 BAR0 |
|  | (512 KiB)    |
|  +--------------+
|
v
```

**Where BARs are located**

Let's locate ourselves globally within the PCIe environment.

For each PCIe device, this is how things are logically organized hierarchically:

```
+--------+                  +------------+       +------+       +-------------+
| device |>---------------->| function 0 |>----->| BAR0 |>----->| BAR0 region |
| xx:yy  |                  | xx:yy.0    |       +------+       +-------------+
|        |>------------+    |            |
|        |             |    |            |       +------+       +-------------+
   ...        ...      |    |            |>----->| BAR1 |>----->| BAR1 region |
|        |             |    |            |       +------+       +-------------+
|        |>--------+   |    |            |
+--------+         |   |         ...        ...    ...
                   |   |    |            |
                   |   |    |            |       +------+       +-------------+
                   |   |    |            |>----->| BAR5 |>----->| BAR5 region |
                   |   |    +------------+       +------+       +-------------+
                   |   |
                   |   |
                   |   |    +------------+       +------+       +-------------+
                   |   +--->| function 1 |>----->| BAR0 |>----->| BAR0 region |
                   |        | xx:yy.1    |       +------+       +-------------+
                   |        |            |               
                   |        |            |       +------+       +-------------+
                   |        |            |>----->| BAR1 |>----->| BAR1 region |
                   |        |            |       +------+       +-------------+
                   |        |            |               
                   |             ...        ...    ...   
                   |        |            |               
                   |        |            |       +------+       +-------------+
                   |        |            |>----->| BAR5 |>----->| BAR5 region |
                   |        +------------+       +------+       +-------------+
                   |
                   |
                   |             ...
                   |
                   |
                   |        +------------+       +------+       +-------------+
                   +------->| function 7 |>----->| BAR0 |>----->| BAR0 region |
                            | xx:yy.7    |       +------+       +-------------+
                            |            |               
                            |            |       +------+       +-------------+
                            |            |>----->| BAR1 |>----->| BAR1 region |
                            |            |       +------+       +-------------+
                            |            |               
                                 ...        ...    ...   
                            |            |               
                            |            |       +------+       +-------------+
                            |            |>----->| BAR5 |>----->| BAR5 region |
                            +------------+       +------+       +-------------+
```

Each PCIe device can define a total of 1 to 8 "functions". Each function can be identified by the 2-byte "bus-device-function" (BDF) triplet:

* bus: 8 bits (max 256), which bus the PCIe device is connected to
* device: 5 bits (max 32 per bus), which device of the bus
* function: 3 bits (max 8 per device), which function of the device

These three values are often represented in the format

```
<bus>:<device>.<function>
```

e.g. doing `lspci` on my Lenovo ThinkPad P14s laptop gives among other lines:

```
00:00.0 Host bridge: Advanced Micro Devices, Inc. [AMD] Device 14e8
00:00.2 IOMMU: Advanced Micro Devices, Inc. [AMD] Device 14e9
00:01.0 Host bridge: Advanced Micro Devices, Inc. [AMD] Device 14ea
00:02.0 Host bridge: Advanced Micro Devices, Inc. [AMD] Device 14ea
00:02.1 PCI bridge: Advanced Micro Devices, Inc. [AMD] Device 14ee
00:02.2 PCI bridge: Advanced Micro Devices, Inc. [AMD] Device 14ee
00:02.4 PCI bridge: Advanced Micro Devices, Inc. [AMD] Device 14ee
64:00.0 VGA compatible controller: Advanced Micro Devices, Inc. [AMD/ATI] Phoenix1 (rev dd)
64:00.1 Audio device: Advanced Micro Devices, Inc. [AMD/ATI] Rembrandt Radeon High Definition Audio Controller
64:00.2 Encryption controller: Advanced Micro Devices, Inc. [AMD] Family 19h (Model 74h) CCP/PSP 3.0 Device
64:00.3 USB controller: Advanced Micro Devices, Inc. [AMD] Device 15b9
64:00.4 USB controller: Advanced Micro Devices, Inc. [AMD] Device 15ba
64:00.5 Multimedia controller: Advanced Micro Devices, Inc. [AMD] ACP/ACP3X/ACP6x Audio Coprocessor (rev 63)
65:00.0 Non-Essential Instrumentation [1300]: Advanced Micro Devices, Inc. [AMD] Device 14ec
65:00.1 Signal processing controller: Advanced Micro Devices, Inc. [AMD] Device 1502
```

so e.g. here we see that

* bus `00` contains several devices such as:
  * `00:00`, which contains two functions:
    * `00:00.0`
    * `00:00.2`
  * `00:01` which has just one mandatory function `00:01.0`
  * `00:02` which has four functions:
    * `00:02.0`
    * `00:02.1`
    * `00:02.2`
    * `00:02.4`

and so on for bus `64`.

Each function has one "PCIe configuration space", which a standardized small chunk of memory used to get information or do certain operations common to all devices.

For PCI Type 0 (Non-Bridge) devices, the configuration space looks like this:

```
0                                  15 16                                 31
+------------------+-----------------+------------------+------------------+
| Vendor ID                          | Device ID                           | 00
+------------------+-----------------+------------------+------------------+
| Command                            | Status                              | 04
+------------------+-----------------+------------------+------------------+
| Revision ID                        | Class Code                          | 08
+------------------+-----------------+------------------+------------------+
| Cache Line Size  | Latency Timer   | Header Type      | BIST             | 0C
+------------------+-----------------+------------------+------------------+
| BAR0                                                                     | 10
+------------------+-----------------+------------------+------------------+
| BAR1                                                                     | 14
+------------------+-----------------+------------------+------------------+
| BAR2                                                                     | 18
+------------------+-----------------+------------------+------------------+
| BAR3                                                                     | 1C
+------------------+-----------------+------------------+------------------+
| BAR4                                                                     | 20
+------------------+-----------------+------------------+------------------+
| BAR5                                                                     | 24
+------------------+-----------------+------------------+------------------+
| Cardbus CIS Pointer                                                      | 28
+------------------+-----------------+------------------+------------------+
| Subsystem Vendor ID                | Subsystem ID                        | 2C
+------------------+-----------------+------------------+------------------+
| Expansion ROM Base Address                                               | 30
+------------------+-----------------+------------------+------------------+
| Cap. Pointer     |                                                       | 34
+------------------+-----------------+------------------+------------------+
|                                                                          | 38
+------------------+-----------------+------------------+------------------+
| Interrupt Line   | Interrupt Pin   | Min Gnt.         | Max Lat.         | 3C
+------------------+-----------------+------------------+------------------+
```

Adapted from: https://en.wikipedia.org/wiki/File:Pci-config-space.svg

Therefore we see that each such "PCIe function" has 6 32-bit BAR registers, BAR0 to BAR5, allowing it to register up to 6 BAR regions for itself. Each BAR can also be marked as inactive TODO how, all zeroes? Or width 0?

Therefore, each device can have up to 48 BAR registers: 8 functions times 6 BARs per function.

To give some context, other notable things present in the configuration space besides the BARs include:

* the 16 bit vendor ID and device ID, which allows the operating system to identify the device and select an appropriated driver for it
* master enable bit (bit 2) of the command register allows enabling/disabling the entire PCIe device

Note that like the BAR registers, these operations are common to all PCIe devices. This is in opposition to writing to the BAR regions, which is highly device specific.

The beauty of the standardized configuration space is that it offers a single standardized interface for the kernel can query to decide which device it is, or do common device operations, forwarding more device-specific operations to the required drivers and device-specific BAR regions.

**Layout of each BAR register**

The [wiki page](https://en.wikipedia.org/wiki/PCI_configuration_space) gives the layout of each bar.

For "memory"-type BARs:

| Bits | Description | Values |
| --- | --- | --- |
| 0 | Region Type | 0 = memory<br/>1 = I/O (deprecated) |
| 2-1 | Locatable | 0 = any 32-bit<br/>1 = \< 1 MiB<br/>2 = any 64-bit |
| 3 | Prefetchable | 0 = no<br/>1 = yes |
| 31-4 | Base Address | 16-byte aligned |

and for "I/O" type BARs, there's just one big 31-bit Base Address field after the "Region Type" bit.

The most important field in the "memory"-type bar is the 27-bit "Base Address" field, which determines the initial address in memory of the corresponding BAR region, in multiples of 16-bytes.

From this we see that the maximum address of the start of a range is when all bits are '1' so:

```
(2^28 - 1) * 2^4 = 4 GiB - 16
```

[Wikipedia however mentions](https://en.wikipedia.org/wiki/PCI_configuration_space) that there is an option to have base addresses above 4 GiB:

> If a platform supports the "Above 4G" option in system firmware, 64 bit BARs can be used.

**Finding the region width**

On the layout table, there is no mention of the region width, only the base address.

This is because determining the region width requires you to first write 1's to the BAR.

This modifies its value, so that when you next read it the result gives you the width: https://stackoverflow.com/questions/19006632/how-is-a-pci-pcie-bar-size-determined

This gives you a certain number or leading 1's, which maps to a power of two size between 16 bytes and 2 GiB.

**Determine BAR regions on Linux**

Running:

```
lspci -vv
```

clearly gives it to us on the "Region" entries of each PCI efunction. E.g. on my Lenovo ThinkPad P14s and slightly manually edited for brevity I see:

```
64:00.0 VGA compatible controller: Advanced Micro Devices, Inc. [AMD/ATI] Phoenix1 (rev dd) (prog-if 00 [VGA controller])                                                                     
        Region 0: Memory at 2400000000 (64-bit, prefetchable) [size=256M]
        Region 2: Memory at 78000000 (64-bit, prefetchable) [size=2M]
        Region 4: I/O ports at 1000 [size=256]
        Region 5: Memory at 78500000 (32-bit, non-prefetchable) [size=512K]

64:00.1 Audio device: Advanced Micro Devices, Inc. [AMD/ATI] Rembrandt Radeon High Definition Audio Controller
        Region 0: Memory at 785c8000 (32-bit, non-prefetchable) [size=16K]

64:00.2 Encryption controller: Advanced Micro Devices, Inc. [AMD] Family 19h (Model 74h) CCP/PSP 3.0 Device
        Region 2: Memory at 78400000 (32-bit, non-prefetchable) [size=1M]
        Region 5: Memory at 785cc000 (32-bit, non-prefetchable) [size=8K]
```

so we understand that:

* function 64:00.0 has 4 BAR regions setup, 0, 2, 4, and 5. BAR0 for example starts at memory address 2400000000 (144 GiB, so I must have "Above 4G"), and has a width of 256MB. The entry also tells us that the "Prefetchable" bit is set to 1.
* function 64:00.1 has a single region region BAR0
* function 64:00.2 has 2 BAR regions setup: BAR2 and BAR5

**Play with it with QEMU and the Linux kernel**

A good way to learn something is to interact with it, and the perfect setup for that is with the Linux kernel on QEMU with the QEMU "edu device"!

QEMU does not model PCIe at the TLP level; [data just magically flies from memory to devices and back](https://stackoverflow.com/questions/71706706/qemu-pcie-tlp-emulation). But all the software-level details are clearly visible so that's OK.

The QEMU "edu" device is an educational PCIe device, and therefore perfect for our purposes. It is clearly documented at: https://www.qemu.org/docs/master/specs/edu.html

And if anything is unclear, the source is at: https://github.com/qemu/qemu/blob/760b4dcdddba4a40b9fa0eb78fdfc7eda7cb83d0/hw/misc/edu.c

That device [registers a single 1 MiB BAR region](https://github.com/qemu/qemu/blob/760b4dcdddba4a40b9fa0eb78fdfc7eda7cb83d0/hw/misc/edu.c#L387)

```
memory_region_init_io(&edu->mmio, OBJECT(edu), &edu_mmio_ops, edu,
                    "edu-mmio", 1 * MiB);
pci_register_bar(pdev, 0, PCI_BASE_ADDRESS_SPACE_MEMORY, &edu->mmio);
```

and then the docs https://www.qemu.org/docs/master/specs/edu.html explain what reads and writes to the BAR region do https://www.qemu.org/docs/master/specs/edu.html#mmio-area-spec e.g.:

* 0x08: if you write to it, it gets replaced with its factorial which can then be read
* 0x60: raises an interrupt if you write to it
* 0x80, 0x88, 0x90 and 0x98 configure and start DMA

To enable the edu device it you have to launch QEMU with:

```
-device edu
```

**Linux kernel PCIe interaction**

The Linux kernel has extensive PCIe interaction facilities of course, given that it is used to interface with so many super important different PCIe hardware.

Here is a minimal kernel driver example I've created for edu: https://github.com/cirosantilli/linux-kernel-module-cheat/blob/366b1c1af269f56d6a7e6464f2862ba2bc368062/kernel_module/pci.c

Also that hole repo of mine automates building the root image for you with Buildroot, and cross compiling the kernel modules esaily: https://github.com/cirosantilli/linux-kernel-module-cheat

Several Linux kernel PCI functions take the `BAR` as a parameter to identify which communication channel is to be used, e.g.:

```
mmio = pci_iomap(pdev, BAR, pci_resource_len(pdev, BAR));
pci_resource_flags(dev, BAR);
pci_resource_start(pdev, BAR);
pci_resource_end(pdev, BAR);
```

Depending on the BAR "Region Type" (0/1), we should use different functions to read and write to the BAR region:

- `IORESOURCE_IO`: must be accessed with `inX` and `outX`
- `IORESOURCE_MEM`: must be accessed with `ioreadX` and `iowriteX`

Bibliography:

* http://wiki.osdev.org/PCI#Base_Address_Registers of course.

[A CPU walks into a BAR... it orders Two Large Pints](https://en.wikipedia.org/wiki/Bar_joke).