cmake_minimum_required(VERSION 3.10)
project(Huffman)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

add_subdirectory(gflags)
add_subdirectory(tests)
add_subdirectory(lib)

set(CMAKE_CXX_STANDARD 17)

add_executable(Huffman main.cpp )
target_link_libraries(Huffman gflags::gflags HuffmanLibrary)

if (MSVC)
    target_compile_options(Huffman PRIVATE /W4 /WX)
else()
    target_compile_options(Huffman PRIVATE -Wall -Wextra -Werror)
endif()
