#!/bin/bash

set -e

./build.sh
./gdb_run.sh bin/kernel_debug.elf
