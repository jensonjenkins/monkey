#include "../src/lexer.hpp"
#include "../src/parser.hpp"
#include <iostream>
#include <cstdlib>
#include <string>

bool check_parser_errors(parser::parser p) {
    std::vector<std::string> errors = p.errors();
    if(errors.size() == 0) { return true; }
    std::cout<<"parser errors: "<<std::endl;
    for(std::string_view error : errors) {
        std::cout<<"\t"<<error<<std::endl;
    }
    return false;
}

bool parser::trace::_enable_trace = 0;
size_t parser::trace::_indent_level = 0;

int main() {
    std::cout<<"Monkey v0.0.1 (main, REPL)"<<std::endl;
    while(1) {
        std::cout<<">>> ";

        std::string input_str;
        std::getline(std::cin, input_str);
        const char* input = input_str.c_str();

        lexer::lexer l(input);
        parser::parser p(l);
        ast::program* program = p.parse_program();
        if(!check_parser_errors(p)) {
            continue;
        }

        std::cout<<program->to_string()<<std::endl;
    }
    exit(EXIT_SUCCESS);
}