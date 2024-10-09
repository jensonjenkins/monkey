cmake_minimum_required(VERSION "3.10")
project("monkey")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(src)

add_executable(lexer_test tests/lexer_test.cpp)
add_executable(parser_test tests/parser_test.cpp)


