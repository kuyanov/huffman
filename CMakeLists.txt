cmake_minimum_required(VERSION 3.10)
project(huffman)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address,undefined -fno-sanitize-recover=all")

add_subdirectory(gflags)
add_subdirectory(tests)
add_subdirectory(lib)

add_executable(Huffman main.cpp )
target_link_libraries(Huffman gflags::gflags HuffmanLibrary)

if (MSVC)
    target_compile_options(Huffman PRIVATE /W4 /WX)
else()
    target_compile_options(Huffman PRIVATE -Wall -Wextra -Werror)
endif()
