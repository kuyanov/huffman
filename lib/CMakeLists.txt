project(HuffmanLib)

SET(CMAKE_CXX_STANDARD 17)

set(SRCS
    Huffman.cpp
    Huffman.h
)

add_library(HuffmanLibrary STATIC ${SRCS})
target_link_libraries(HuffmanLibrary )
target_include_directories(HuffmanLibrary PUBLIC ./)

if(MSVC)
  target_compile_options(HuffmanLibrary PRIVATE /W4 /WX)
else()
  target_compile_options(HuffmanLibrary PRIVATE -Wall -Wextra -Werror)
endif()