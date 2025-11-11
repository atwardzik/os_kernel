add_library(klibc OBJECT
        kstdio.c
        kstring.c
)

target_include_directories(klibc PUBLIC ${CMAKE_SOURCE_DIR}/include/klibc)
target_include_directories(klibc PUBLIC ${CMAKE_SOURCE_DIR}/include)