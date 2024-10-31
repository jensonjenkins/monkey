#include "../src/lexer.hpp"
#include "../src/object.hpp"
#include "../src/evaluator.hpp"
#include "../src/parser.hpp"
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>

bool check_parser_errors(parser::parser p) {
    std::vector<std::string> errors = p.errors();
    if(errors.size() == 0) { return true; }
    std::cout<<"parser errors: "<<std::endl;
    for(std::string_view error : errors) {
        std::cout<<"\t"<<error<<std::endl;
    }
    return false;
}

bool    parser::trace::_enable_trace = 0;
size_t  parser::trace::_indent_level = 0;

int main() {
    std::string input;
    std::ostringstream oss;

    auto s = std::chrono::high_resolution_clock::now();

    while (std::getline(std::cin, input)) {
        oss << input << "\n"; 
    }

    std::string full_input = oss.str();
    const char* cstr_input = full_input.c_str();

    lexer::lexer l(cstr_input);
    parser::parser p(l);
    std::shared_ptr<const ast::program> program(p.parse_program());
    if(!check_parser_errors(p)) {
        exit(EXIT_FAILURE);
    }

    object::scope* scope = new object::scope();
    object::object* evaluated = evaluator::eval(program, scope);
    if(evaluated != nullptr){
        std::cout<<evaluated->inspect()<<std::endl;
    }

    auto e = std::chrono::high_resolution_clock::now();
    std::cout
        << "time: "
        << std::chrono::duration_cast<std::chrono::microseconds>(e - s).count()
        << " micro s"
        << std::endl;
 
    return 0;
}
