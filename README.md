![Arch RP2040](https://img.shields.io/badge/Arch-RP2040_(coming_soon)-red?style=flat-square)
![Arch RP2350](https://img.shields.io/badge/Arch-RP2350-green?style=flat-square)

# Simple kernel for RP2040 and RP2350

Baremetal code, written for Raspberry Pi Pico. Primarily, intentionally written with as much ARM assembly as possible.
As of now the kernel is so complex that using assembly in areas other than hardware drivers is
highly unsafe and not recommended.

## Features

- Multiple drivers for peripherals written in assembly 🚧
- Dynamic memory allocation ✅
- Dedicated simple libc for kernel space ✅
- Cooperative and Pre-emptive Multitasking 🚧
- Multiple syscalls in the POSIX compatibility ✅
- Filesystem ✅
- EXT2 support 🚧

## Drivers

- PS2 keyboard driver ✅
- UART communication ✅
- VGA monitor driver 640x480 ✅
- SD card driver 🚧
- Ethernet WIZnet W5100S 🚧

## Future plans

- Porting LIBC to user space programs  as a dynamic library
- Writing user-space programs like CLI, assembler and vim-like editor
- Introducing Rust into kernel
- ELF executable support
- Multithreading
- Memory Protection
- After full filesystem support move some parts of firmware into bootloader. The kernel and the initramfs should reside in the `/boot` directory

## License

Open-source and released under the BSD 3-Clause License. Feel free to use, modify, and distribute the code in accordance
with the terms specified in the license.

Copyright (C) 2025 Artur Twardzik
