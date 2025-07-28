cmake_minimum_required(VERSION 3.30)
set(CMAKE_TOOLCHAIN_FILE pico_toolchain.cmake)
set(LINKER_SCRIPT "linker.ld")

if (DEFINED ARCH_RP2040)
    set(PREPROCESSOR_DEFINE "-D ARCH_RP2040=1")
elseif (DEFINED ARCH_RP2350)
    set(PREPROCESSOR_DEFINE "-D ARCH_RP2350=1")
else ()
    message(FATAL_ERROR "Target architecture should be defined, add -DARCH_RP2040 or -DARCH_RP2350")
endif ()

project(kernel LANGUAGES C ASM)

include_directories(${CMAKE_SOURCE_DIR} include tests)
link_directories(${CMAKE_SOURCE_DIR} include tests)

preprocess_sources(${PREPROCESSOR_DEFINE}
        src/drivers/gpio_functions.S
        src/drivers/time.S
        src/drivers/uart.S
        src/drivers/keyboard.S
        src/drivers/pio.S
        src/drivers/vga.S
        src/resets.S
        src/divider.S
)

add_executable(kernel
        ${PREPROCESS_OUTPUTS}
        src/start.s
        src/isr.s
        src/main.c
        src/stdio.s
        src/proc.c
        src/memory.c
)


set_source_files_properties(src/memory.c PROPERTIES COMPILE_FLAGS -Wno-pointer-arith)
set_source_files_properties(src/proc.c PROPERTIES COMPILE_FLAGS -Wno-pointer-arith)

target_link_libraries(kernel
        -T${LINKER_SCRIPT}
        -nostdlib
)

set_target_properties(kernel PROPERTIES
        LINK_DEPENDS ${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT}
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
        OUTPUT_NAME "kernel_debug.elf"
)
