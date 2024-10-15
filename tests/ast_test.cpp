#include <cstdlib>
#include <iostream>
#include "../src/ast.hpp"
#include "../src/token.hpp"

namespace ast {

void test_to_string() {
    program p;
    let_statement* ls = new let_statement(token::token(token::LET, "let"));
    ls->move_ident(identifier(token::token(token::IDENT, "my_var"), "my_var"));
    identifier* expr = new identifier(token::token(token::IDENT, "another_var"), "another_var");
    ls->move_value(expr);
    p.add_statement(ls);

    if(p.to_string() != "let my_var = another_var;\n"){
        std::cout<<"program to_string() wrong. got "<<p.to_string()<<std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout<<"1 - ok: let_statement to_string ok."<<std::endl;
}

} // namespace ast 

int main() {
    std::cout<<"Running ast_test.cpp..."<<std::endl;
    
    ast::test_to_string();

    std::cout<<"ast_test.cpp: ok"<<std::endl;

    exit(EXIT_SUCCESS);
}

