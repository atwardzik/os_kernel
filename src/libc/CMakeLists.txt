add_library(libc OBJECT
        stdio.s
        stdio.c
)

target_include_directories(libc PUBLIC ${CMAKE_SOURCE_DIR}/include)
