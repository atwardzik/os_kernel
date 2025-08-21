add_library(kernel OBJECT
        memory.c
        proc.c
        ctx.s
        isr.s
)

set_source_files_properties(memory.c PROPERTIES COMPILE_FLAGS -Wno-pointer-arith)
set_source_files_properties(proc.c PROPERTIES COMPILE_FLAGS -Wno-pointer-arith)

target_include_directories(kernel PUBLIC ${CMAKE_SOURCE_DIR}/include/kernel)
target_include_directories(kernel PUBLIC ${CMAKE_SOURCE_DIR}/include)
