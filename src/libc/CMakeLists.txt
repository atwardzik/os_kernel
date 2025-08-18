add_library(libc STATIC
        stdio.s
)

target_include_directories(libc PUBLIC ${CMAKE_SOURCE_DIR}/include)
