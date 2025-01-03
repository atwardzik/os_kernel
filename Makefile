.PHONY: uf2
.PHONY: clean

ARM ?= arm-none-eabi
AS_ARGS = --warn --fatal-warnings -mcpu=cortex-m0 -mthumb -g
GCC_ARGS = -Wall -Wextra -Wpedantic -Werror -Wno-unused-variable -Iinclude -std=c2x -mcpu=cortex-m0 -ffreestanding -mthumb -c -g -O0
LINK_ARGS = -nostdlib

all: start.o main.o gpio_functions.o time.o uart.o stdio.o resets.o proc.o memory.o debug

clean:
	@rm -f build/*.o bin/*.elf

######################### DRIVERS ##########################
time.o: src/drivers/time.s
	$(ARM)-as $(AS_ARGS) src/drivers/time.s -o build/time.o

gpio_functions.o: src/drivers/gpio_functions.s
	$(ARM)-as $(AS_ARGS) src/drivers/gpio_functions.s -o build/gpio_functions.o

uart.o: src/drivers/uart.s
	$(ARM)-as $(AS_ARGS) src/drivers/uart.s -o build/uart.o

######################### SOURCES ##########################
start.o: src/start.s
	$(ARM)-as $(AS_ARGS) src/start.s -o build/start.o

resets.o: src/resets.s
	$(ARM)-as $(AS_ARGS) src/resets.s -o build/resets.o

stdio.o: src/stdio.s
	$(ARM)-as $(AS_ARGS) src/stdio.s -o build/stdio.o

proc.o: src/proc.c
	$(ARM)-gcc $(GCC_ARGS) src/proc.c -o build/proc.o

memory.o: src/memory.c
	$(ARM)-gcc -Wno-pointer-arith $(GCC_ARGS) src/memory.c -o build/memory.o

main.o: src/main.c
	$(ARM)-gcc $(GCC_ARGS) src/main.c -o build/main.o

######################### TARGET ##########################
debug: start.o gpio_functions.o time.o uart.o stdio.o resets.o proc.o memory.o main.o
	$(ARM)-ld $(LINK_ARGS) -T linker.ld build/*.o -o bin/debug.elf

