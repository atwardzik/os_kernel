#!/bin/bash
arm-none-eabi-gdb --quiet \
    -ex "target remote localhost:3333" \
    -ex "monitor reset init" \
    -ex "load" \
    -ex "break main" \
    -ex "continue" \
    -ex "layout regs" $1
#		-ex "layout split" \
#    -ex "layout regs" $1
