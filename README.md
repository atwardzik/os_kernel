# Simple kernel for RP2040 and RP2350

Baremetal code, written for Raspberry Pi Pico. Intentionally written with as much ARM assembly as possible.

## Features
 - Multiple drivers for peripherals written in assembly 🚧
 - Dynamic memory allocation ✅
 - Porting newlibc to kernel space ✅
 - Cooperative and Pre-emptive Multitasking 🚧
 - Multiple syscalls in the POSIX compatibility 🚧
 - Filesystem 🚧

## Drivers
 - PS2 keyboard driver ✅
 - UART communication ✅
 - VGA monitor driver 640x480 ✅
 - SD card driver 🚧 
 - Ethernet ENC28J60 🚧

## Future plans
- Porting LIBC to user space programs
- Writing user-space programs like CLI, assembler and vim-like editor
- Introducing Rust into kernel
- ELF executable support

## License
Open-source and released under the BSD 3-Clause License. Feel free to use, modify, and distribute the code in accordance with the terms specified in the license.

Copyright (C) 2025 Artur Twardzik
