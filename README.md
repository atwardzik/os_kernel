# Simple kernel for RP2040 and RP2350

Baremetal code, written for Raspberry Pi Pico. Intentionally written with as much ARM assembly as possible.

## Features
 - Multiple drivers for peripherals written in assembly 🚧
 - Dynamic memory allocation ✅
 - Cooperative and Pre-emptive Multitasking 🚧
 - Multiple syscalls 🚧
 - Filesystem 🚧

## Drivers
 - PS2 keyboard driver ✅
 - UART communication ✅
 - VGA monitor driver 640x480 and 800x600 🚧

## Future plans
- Porting LIBC
- Writing user-space programs like CLI, assembler and vim-like editor
- Introducing Rust into kernel
- ELF executable support

## License
Open-source and released under the BSD 3-Clause License. Feel free to use, modify, and distribute the code in accordance with the terms specified in the license.

Copyright (C) 2025 Artur Twardzik
