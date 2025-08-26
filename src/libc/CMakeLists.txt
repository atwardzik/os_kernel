add_library(libc OBJECT
        stdio.S
        printer.c
)

target_include_directories(libc PUBLIC ${CMAKE_SOURCE_DIR}/include)
