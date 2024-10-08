#include <cstdlib>
#include <iostream>

#include "../src/parser.hpp"

namespace parser {

void test_let_statement_1() {
    const char* input = R"(
    let x = 5;
    let y = 10;
    let foobar = 123134;
    )";
    
    const char* var_names[] = {"x", "y", "foobar"};
    lexer::lexer l(input);
    parser p(l);
    
    ast::program* program = p.parse_program();

    if(program == nullptr) {
        std::cout<<"test_let_statement_1 - parse_program() returned nullptr."<<std::endl;
    }
    if(program->_statements.size() != 3) {
        std::cout<<"test_let_statement_1 - incorrect number of program statements."<<std::endl;
    }

    auto test_let_statement = [](ast::statement* s, const char* name) -> bool {

        ast::let_statement* ls = dynamic_cast<ast::let_statement*>(s);

        if(ls == nullptr) {
            std::cout<<"test_let_statement_1 - s is not an ast::let_statement."<<std::endl;
            return false;
        }
        if(s->token_literal() != "let") {
            std::cout<<"test_let_statement_1 - token_literal() not let."<<std::endl;
            return false;
        }
        if(ls->get_ident().token_literal() != name) {
            std::cout<<"test_let_statement_1 - s identifier token_literal not equal to "<<name<<std::endl;
            return false;
        }
        if(ls->get_ident().get_value() != name) {
            std::cout<<"test_let_statement_1 - s identifier name not equal to "<<name<<std::endl;
            return false;
        }
        return true;
    };
    
    for(int i=0;i<3;i++){
        ast::statement* stmt = program->get_statements()[i].get();

        if(!test_let_statement(stmt, var_names[i])) { exit(EXIT_FAILURE); }
    }

    std::cout<<"1 - ok: parse let statements with int rvalues."<<std::endl;
}

} //namespace parser


int main(){
    std::cout<<"Running parser_test.cpp..."<<std::endl;
    
    parser::test_let_statement_1();

    exit(EXIT_SUCCESS);
}

