cmake_minimum_required(VERSION 3.15)
project(edu25519 C)

set(CMAKE_C_STANDARD 11)

file(GLOB sources
        "src/*.h"
        "src/*.c"
)

add_library(edu25519 STATIC ${sources})

add_executable(example example.c)
target_link_libraries(example edu25519)
