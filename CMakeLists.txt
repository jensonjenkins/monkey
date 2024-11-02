cmake_minimum_required(VERSION "3.10")
project(monkey LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include_directories(src)

set(CMAKE_BUILD_TYPE Debug)

add_executable(lexer_test tests/lexer_test.cpp)
add_executable(ast_test tests/ast_test.cpp)
add_executable(parser_test tests/parser_test.cpp)
add_executable(evaluator_test tests/evaluator_test.cpp)
add_executable(repl monkey/repl.cpp)
add_executable(monkey monkey/monkey.cpp)

set_target_properties(lexer_test PROPERTIES COMPILE_FLAGS "-g")
set_target_properties(ast_test PROPERTIES COMPILE_FLAGS "-g")
set_target_properties(parser_test PROPERTIES COMPILE_FLAGS "-g")
set_target_properties(evaluator_test PROPERTIES COMPILE_FLAGS "-g")
