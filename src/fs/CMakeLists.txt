add_library(fs OBJECT
        file.c
)

target_include_directories(fs PUBLIC ${CMAKE_SOURCE_DIR}/include/fs)
target_include_directories(fs PUBLIC ${CMAKE_SOURCE_DIR}/include)
