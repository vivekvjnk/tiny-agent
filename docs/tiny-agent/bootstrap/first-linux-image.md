# Tiny Agent Bootstrap: Building and Booting First Linux Image

*(using Yocto Project + QEMU + Docker Engine on Fedora Linux)*

## 1. Objective

Document how we:

* created containerized Yocto build environment
* built first bootable image
* configured host/container path consistency
* resolved Docker bridge networking corruption
* resolved TUN/TAP issues
* resolved `runqemu` interpreter path mismatch
* successfully booted first emulated Linux image

---

## 2. System Architecture

Diagram:

```text id="1"
Host Fedora
 ├── Docker (build environment)
 │    └── poky-dev container
 │         └── bitbake builds image
 │
 ├── Shared workspace mount
 │    └── /home/vivekv/Documents/tiny-agent
 │
 └── Host QEMU execution
      └── runqemu boots image
```

Key principle:

> **Build in container, emulate on host.**

Reason:

* reproducible builds
* host gets access to `/dev/net/tun`
* host networking simpler
* no privileged container requirement

---

## 3. Workspace Layout

```text id="2"
tiny-agent/
└── poky/
    └── build-qemu/
```

Canonical absolute path:

```text id="3"
/home/vivekv/Documents/tiny-agent
```

Important:

> Host path == container mount path

Never:

```text id="4"
/workdir
```

if host uses another absolute path.

---

## 4. Container Setup

Dockerfile
base packages
user mapping
volume mounts
SELinux notes

---

## 5. Building Image

Commands:

```bash id="5"
source oe-init-build-env build-qemu
bitbake core-image-minimal
```

Artifacts produced:

* kernel
* rootfs
* qemuboot.conf

Paths documented

---

## 6. Problems Encountered

### 6.1 Docker bridge corruption after `modprobe tun`

Symptoms
diagnosis
fix sequence

### 6.2 Container cannot access `/dev/net/tun`

Why

### 6.3 Yocto native binary ELF interpreter mismatch

Root cause:

embedded interpreter:

```text id="6"
/workdir/...
```

Fix:

consistent path + rebuild

### 6.4 TMPDIR sanity checker

Why Yocto blocks path moves

---

## 7. Running QEMU

Command:

```bash id="7"
runqemu qemux86-64 nographic
```

Networking model
tap setup
host requirements

---

## 8. Lessons Learned

Design principles captured

---

## 9. Next Step: tiny-agent base image

Minimal packages
boot service
agent daemon

