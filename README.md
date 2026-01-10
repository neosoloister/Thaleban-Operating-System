# Thaleban Operating System

A 32-bit operating system kernel written from scratch in C and Assembly.

## Features
- **Bootloader:** Custom 2-stage bootloader (MBR -> Stage 2).
- **Mode:** Switches from 16-bit Real Mode to 32-bit Protected Mode.
- **Kernel:** Minimal 32-bit kernel written in C.
- **Drivers:** VGA Text Mode driver with scrolling and color support.
- **LibC:** Basic `kprintf` implementation.
- **Build System:** Cross-platform `Makefile` (Linux/Windows) with Python-based image generation.

## Prerequisites

You need the following tools installed and available in your PATH:

1.  **Python 3:** Required for building the OS image (`tools/build_image.py`).
2.  **NASM:** The Netwide Assembler.
3.  **QEMU:** `qemu-system-i386` for emulation.
4.  **GCC Cross-Compiler:** `i686-elf-gcc` and `i686-elf-ld`.
    *   **Linux:** Install via package manager or build from source.
    *   **Windows:** Use pre-built binaries or WSL.
5.  **Make:** `make` build tool (MinGW Make on Windows).

## Building and Running

The `Makefile` is designed to work on both Linux and Windows.

To build the OS image:
```bash
make
```

To build and run in QEMU:
```bash
make run
```

To clean build artifacts:
```bash
make clean
```

## Project Structure
- `src/boot/`: Bootloader assembly code (MBR, Stage 2).
- `src/kernel/`: Kernel source code (`kernel.c`).
- `src/drivers/`: Hardware drivers (VGA, etc.).
- `src/libc/`: Standard library implementation (`kprintf`, string utils).
- `tools/`: Build scripts (`build_image.py`).
- `build/`: Compiled binaries and the final `os-image.bin`.