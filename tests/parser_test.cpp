#include <cstdint>
#include <cstdlib>
#include <iostream>

#include "../src/parser.hpp"

namespace parser {

void check_parser_errors(parser p) {
    std::vector<std::string> errors = p.errors();
    if(errors.size() == 0) { return; }
    std::cout<<"parser has "<<errors.size()<<" errors."<<std::endl;
    for(std::string_view error : errors) {
        std::cout<<"parser error: "<<error<<std::endl;
    }
    exit(EXIT_FAILURE);
}

void test_let_statement() {
    const char* input = R"(
    let x = 5;
    let y = 10;
    let foobar = 123134;
    )";
    
    const char* var_names[] = {"x", "y", "foobar"};
    lexer::lexer l(input);
    parser p(l);
    
    ast::program* program = p.parse_program();
    check_parser_errors(p);

    if(program == nullptr) {
        std::cout<<"test_let_statement_1 - parse_program() returned nullptr."<<std::endl;
    }
    if(program->get_statements().size() != 3) {
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

void test_return_statement() {
    const char* input = R"(
    return 5;
    return 10;
    return 1923123;
    )";
    
    lexer::lexer l(input);
    parser p(l);
    
    ast::program* program = p.parse_program();
    check_parser_errors(p);
    
    if(program->get_statements().size() != 3) {
        std::cout<<"test_return_statement_1 - program statements not equal to 3, got "
            <<program->get_statements().size()<<std::endl;
        exit(EXIT_FAILURE);
    }
    
    for(int i=0;i<3;i++){
        ast::statement* s = program->get_statements()[i].get();
        ast::return_statement* rs = dynamic_cast<ast::return_statement*>(s);
        if(rs == nullptr){
            std::cout
                <<"test_return_statement_1 - statement "
                <<i<<" is not a return statement. got "
                <<s->token_literal()
                <<std::endl;
            exit(EXIT_FAILURE);
        }

        if(rs->token_literal() != "return") {
            std::cout
                <<"test_return_statement_1 - statement "
                <<i<<" token_literal not 'return' . got "
                <<s->token_literal()
                <<std::endl;
            exit(EXIT_FAILURE);
        }
    }

    std::cout<<"2 - ok: parse return statements with int rvalues."<<std::endl;
}

void test_identifier_expression() {
    const char* input = "foobar;";

    lexer::lexer l(input);
    parser p(l);
    
    ast::program* program = p.parse_program();
    check_parser_errors(p);
    
    if(program->get_statements().size() != 1) {
        std::cout<<"test_ident_expr_1 - statements.size() not 1, got "
            <<program->get_statements().size()<<std::endl;
        exit(EXIT_FAILURE);
    }
    
    ast::statement* s = program->get_statements()[0].get();
    ast::expression_statement* es = dynamic_cast<ast::expression_statement*>(s);

    if(es == nullptr) {
        std::cout<<"test_ident_expr_1 - statement not expression statement."<<std::endl;
        exit(EXIT_FAILURE);
    }

    ast::expression* e = es->expr();
    ast::identifier* ident = dynamic_cast<ast::identifier*>(e);
    
    if(ident == nullptr) {
        std::cout<<"test_ident_expr_1 - expression not an identifier."<<std::endl;
        exit(EXIT_FAILURE);
    }

    if(ident->token_literal() != "foobar") {
        std::cout<<"test_ident_expr_1 - token literal not foobar, got "
            <<ident->token_literal()<<std::endl;
        exit(EXIT_FAILURE);
    } 
    std::cout<<"3 - ok: parse expression statement with identifier."<<std::endl;
}

void test_integer_literal_expression() {
    const char* input = "5;";

    lexer::lexer l(input);
    parser p(l);
    ast::program* program = p.parse_program();
    check_parser_errors(p);
    
    if(program->get_statements().size() != 1) {
        std::cout<<"test_int_literal_expr - statements.size() not 1, got "
            <<program->get_statements().size()<<std::endl;
        exit(EXIT_FAILURE);
    }
    
    ast::statement* s = program->get_statements()[0].get();
    ast::expression_statement* es = dynamic_cast<ast::expression_statement*>(s);

    if(es == nullptr) {
        std::cout<<"test_int_literal_expr - statement not expression statement."<<std::endl;
        exit(EXIT_FAILURE);
    }

    ast::expression* e = es->expr();
    ast::int_literal* ident = dynamic_cast<ast::int_literal*>(e);
    
    if(ident == nullptr) {
        std::cout<<"test_int_literal_expr - expression not an int literal."<<std::endl;
        exit(EXIT_FAILURE);
    }

    if(ident->token_literal() != "5") {
        std::cout<<"test_int_literal_expr - token literal not foobar, got "
            <<ident->token_literal()<<std::endl;
        exit(EXIT_FAILURE);
    } 
    
    std::cout<<"4 - ok: parse integer literal rvalue."<<std::endl;
}

void test_integer_literal(ast::expression* il, std::int64_t value) {
    ast::int_literal* int_lit = dynamic_cast<ast::int_literal*>(il);

    if(int_lit == nullptr) {
        std::cout<<"test_integer_literal - expression not an integer literal."<<std::endl;
        exit(EXIT_FAILURE);
    }

    if(int_lit->value() != value) {
        std::cout<<"test_integer_literal - int_lit->value() not "<<value<<", got "<<int_lit->value()<<std::endl;
        exit(EXIT_FAILURE);
    }

    if(int_lit->token_literal() != std::to_string(value)) {
        std::cout<<"test_integer_literal - int_lit token_literal not "<<std::to_string(value)
            <<", got "<<int_lit->token_literal()<<std::endl;
        exit(EXIT_FAILURE);
    }
};

void test_parsing_prefix_expression() {
    struct test_case {
        const char*     input;
        const char*     op;
        std::int64_t    int_value;
        test_case(const char* input, const char* op, std::int64_t int_value) :
            input(input), op(op), int_value(int_value) {};
    };
    std::vector<test_case> prefix_test {
        {"!6;", "!", 6},
        {"-15;", "-", 15}
    };

    for(int i=0;i<2;i++) {
        test_case tc = prefix_test[i];
        lexer::lexer l(tc.input);
        parser p(l);
        ast::program* program = p.parse_program();
        check_parser_errors(p);

        if(program->get_statements().size() != 1) {
            std::cout<<"test_parse_prefix - statements.size() not 1, got "
                <<program->get_statements().size()<<std::endl;
            exit(EXIT_FAILURE);
        }

        ast::statement* s = program->get_statements()[0].get();
        ast::expression_statement* es = dynamic_cast<ast::expression_statement*>(s);

        if(es == nullptr) {
            std::cout<<"test_parse_prefix - statement not expression statement."<<std::endl;
            exit(EXIT_FAILURE);
        }

        ast::expression* e = es->expr();
        ast::prefix_expression* pf = dynamic_cast<ast::prefix_expression*>(e);

        if(pf == nullptr) {
            std::cout<<"test_parse_prefix - expression not a prefix expression."<<std::endl;
            exit(EXIT_FAILURE);
        }

        if(pf->op() != tc.op) {
            std::cout<<"test_parse_prefix - op not "<<tc.op<<", got "<<pf->op()<<std::endl;
            exit(EXIT_FAILURE);
        } 

        test_integer_literal(pf->expr(), tc.int_value);
    }
    std::cout<<"5 - ok: parse prefix with ints."<<std::endl;
}


} //namespace parser


int main(){
    std::cout<<"Running parser_test.cpp..."<<std::endl;
    
    parser::test_let_statement();
    parser::test_return_statement();
    parser::test_identifier_expression();
    parser::test_integer_literal_expression();
    parser::test_parsing_prefix_expression();

    std::cout<<"parser_test.cpp: ok"<<std::endl;

    exit(EXIT_SUCCESS);
}

