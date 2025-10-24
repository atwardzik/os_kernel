add_library(kstdlib OBJECT
        kstdio.c
)

target_include_directories(kstdlib PUBLIC ${CMAKE_SOURCE_DIR}/include)