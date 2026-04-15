# PaydOS
Bare-metal ARM operating systems project for the **Raspberry Pi 2 Model B**, focused on kernel mechanics, memory management, debugging, and hardware-near software design.

## Overview

This repository implements a custom bare-metal OS environment on ARM and covers core operating-systems topics such as:

- boot and early kernel initialization
- exception and interrupt handling
- UART-based kernel output and serial debugging
- thread scheduling
- user/kernel separation
- MMU-based virtual memory and memory protection
- address-space management
- QEMU-based testing and debugging
- deployment to real Raspberry Pi 2B hardware

## Raspberry Pi 2B release

For direct hardware use, prebuilt bootable assets are available in the **Releases** section.

A typical release provides a package such as:

```text
oslab-rpi2b-sdcard.zip
```

with files such as:

```text
boot/kernel7.img
boot/config.txt
install_to_sd.sh
```

To boot the project directly on a Raspberry Pi 2B, use the release package instead of rebuilding everything from source.

## Build and run

Typical workflow:

```bash
make
make qemu
make debug
make clean
```

### Common targets

- `make` — build the project
- `make qemu` — run the system in QEMU
- `make debug` — start a debug-oriented run/setup
- `make clean` — remove build artifacts

## Usage

### 1. Build from source
Use the source tree to inspect, modify, build, and debug the system locally.

### 2. Boot from release
Use the prebuilt assets from **Releases** to deploy the system to a Raspberry Pi 2B more quickly.

## Notes

- Main target board: **Raspberry Pi 2 Model B**
- Main boot image: `kernel7.img`
- Prebuilt bootable packages are published via **Releases**
