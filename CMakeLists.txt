cmake_minimum_required(VERSION 3.30)
set(CMAKE_TOOLCHAIN_FILE pico_toolchain.cmake)
set(LINKER_SCRIPT "linker.ld")

project(debug LANGUAGES C ASM)

include_directories(${CMAKE_SOURCE_DIR} include tests)
link_directories(${CMAKE_SOURCE_DIR} include tests)

add_executable(debug
        src/drivers/gpio_functions.s
        src/drivers/time.s
        src/drivers/uart.s
        src/start.s
        src/isr.s
        src/resets.s
        src/main.c
        src/stdio.s
        src/proc.c
        src/memory.c
)

set_source_files_properties(src/memory.c PROPERTIES COMPILE_FLAGS -Wno-pointer-arith)
set_source_files_properties(src/proc.c PROPERTIES COMPILE_FLAGS -Wno-pointer-arith)

target_link_libraries(debug
        -T${LINKER_SCRIPT}
        -nostdlib
)

set_target_properties(debug PROPERTIES
        LINK_DEPENDS ${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT}
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
        OUTPUT_NAME "debug.elf"
)
