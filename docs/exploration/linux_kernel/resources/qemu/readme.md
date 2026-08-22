## Documentation: Running Minimal AArch64 Linux Headless in QEMU
This guide documents how to successfully boot an AArch64 (ARM64) Alpine Linux ISO on a headless x86_64 Fedora host using QEMU direct-kernel boot.
## 1. The Challenges Encountered
During initial setup, several errors occurred due to architecture and environment mismatches:

* The BIOS Path Error: Passing a web URL (https://linaro.org) to the -bios flag failed because QEMU expects a local file directory path.
* Firmware Directory Mismatch: Debian/Ubuntu file locations (/usr/share/qemu-efi-aarch64/QEMU_EFI.fd) do not match Fedora's layout.
* Headless Display Block: Standard QEMU options try to open a GTK/graphics window. On a headless host, this causes a gtk initialization failed crash.
* Missing Root FS (Kernel Panic): Direct-kernel booting without attaching the backing ISO leaves the image stranded inside an emergency initramfs recovery shell because it cannot locate the core system storage files.

------------------------------
## 2. Step-by-Step Working Solution
The absolute cleanest way to run an ARM64 system over a headless SSH or console line is to extract the kernel assets and map the console outputs directly to your active text terminal frame.
## Step 1: Mount the ISO and Extract Boot Files
Instead of fighting with virtual UEFI flash variables (.fd), mount the ISO image locally to pull out the native kernel binaries:

# 1. Create a local mount point
mkdir -p /tmp/alpine-iso
# 2. Loop-mount your downloaded AArch64 Alpine ISO 
sudo mount -o loop alpine-standard-3.24.1-aarch64.iso /tmp/alpine-iso
# 3. Copy the core Linux kernel and the initial filesystem RAM disk to your workspace
cp /tmp/alpine-iso/boot/vmlinuz-lts .
cp /tmp/alpine-iso/boot/initramfs-lts .
# 4. Safely unmount the ISO structure
sudo umount /tmp/alpine-iso

## Step 2: Launch the Headless Virtual Machine
Run the final verified command block. This tells QEMU to emulate an ARM64 Cortex-A57 processor, handle kernel operations directly in memory, link the configuration to your serial console, and provide the ISO path for package retrieval:

qemu-system-aarch64 \
  -M virt \
  -cpu cortex-a57 \
  -m 2G \
  -smp 2 \
  -nographic \
  -kernel ./vmlinuz-lts \
  -initrd ./initramfs-lts \
  -append "console=ttyAMA0" \
  -cdrom alpine-standard-3.24.1-aarch64.iso

------------------------------
## 3. Explaining Key Technical Flags

* -M virt: Simulates a non-specific development board setup. This is mandatory for ARM compilation testing since ARM platforms do not have generic PC motherboards.
* -cpu cortex-a57: Simulates a standard 64-bit ARM processor instruction profile inside an x86 software translation environment.
* -nographic: Suppresses video windows entirely and stops display engine generation.
* -kernel & -initrd: Pulls the core startup engine files directly into virtual space, skipping any complex intermediate bootloaders.
* -append "console=ttyAMA0": Tells the inner Linux kernel to pipe its internal system startup streams explicitly into QEMU's primary text interface.
* -cdrom: Delivers the root filesystem contents so the kernel doesn't drop you into an emergency recovery prompt.

------------------------------
## 4. Operational Tips Inside the VM

* System Entry: When the alpine login: message displays, input root (no password setup required).
* Graceful Exit: To terminate the emulation thread completely, hold down Ctrl + A, then let go and hit C to reach the QEMU control line. Type quit and press Enter.

