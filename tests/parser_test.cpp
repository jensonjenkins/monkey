#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <type_traits>

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

void test_integer_literal(ast::expression* il, std::int64_t value) {
    ast::int_literal* int_lit = dynamic_cast<ast::int_literal*>(il);

    if(int_lit == nullptr) {
        std::cout<<"fail: test_integer_literal - expression not an integer literal."<<std::endl;
        exit(EXIT_FAILURE);
    }

    if(int_lit->value() != value) {
        std::cout<<"fail: test_integer_literal - int_lit->value() not "<<value<<", got "<<int_lit->value()<<std::endl;
        exit(EXIT_FAILURE);
    }

    if(int_lit->token_literal() != std::to_string(value)) {
        std::cout<<"fail: test_integer_literal - int_lit token_literal not "<<std::to_string(value)
            <<", got "<<int_lit->token_literal()<<std::endl;
        exit(EXIT_FAILURE);
    }
}

void test_identifier(ast::expression* expr, const std::string& value) {
    ast::identifier* ident = dynamic_cast<ast::identifier*>(expr);
    if(ident == nullptr) {
        std::cout<<"fail: test_identifier - expression not an identifier."<<std::endl;
        exit(EXIT_FAILURE);
    }

    if(ident->value() != value) {
        std::cout<<"fail: test_identifier - ident->get_value() not "<<value<<", got "<<ident->value()<<std::endl;
        exit(EXIT_FAILURE);
    }

    if(ident->token_literal() != value) {
        std::cout<<"fail: test_identifier - ident token_literal not "<<value
            <<", got "<<ident->token_literal()<<std::endl;
        exit(EXIT_FAILURE);
    }
}

template <typename T>
void test_literal_expression(ast::expression* expr, T v) {
    if constexpr(std::is_same_v<T, int> || std::is_same_v<T, std::int64_t>) {
        return test_integer_literal(expr, v);
    } else if constexpr (std::is_same_v<T, std::string>) {
        return test_identifier(expr, std::string(v));
    };
    std::cout<<"fail: test_literal_expression - type of expression not handled."<<std::endl;
    exit(EXIT_FAILURE);
}

template <typename L, typename R>
void test_infix_expression(ast::expression* expr, L left, std::string op, R right) {
    ast::infix_expression* ie = dynamic_cast<ast::infix_expression*>(expr);

    if(ie == nullptr) {
        std::cout<<"fail: test_infix_expression - expression not a infix expression."<<std::endl;
        exit(EXIT_FAILURE);
    }

    test_literal_expression(ie->l_expr(), left);

    if(ie->op() != op) {
        std::cout<<"fail: test_infix_expression - op not "<<op<<", got "<<ie->op()<<std::endl;
        exit(EXIT_FAILURE);
    } 

    test_literal_expression(ie->r_expr(), right);
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
        std::cout<<"fail: test_let_statement - parse_program() returned nullptr."<<std::endl;
    }
    if(program->statements().size() != 3) {
        std::cout<<"fail: test_let_statement - incorrect number of program statements."<<std::endl;
    }
    
    auto test_let_statement = [](ast::statement* s, const char* name) -> bool {
        ast::let_statement* ls = dynamic_cast<ast::let_statement*>(s);

        if(ls == nullptr) {
            std::cout<<"fail: test_let_statement - s is not an ast::let_statement."<<std::endl;
            return false;
        }
        if(s->token_literal() != "let") {
            std::cout<<"fail: test_let_statement - token_literal() not let."<<std::endl;
            return false;
        }
        if(ls->ident().token_literal() != name) {
            std::cout<<"fail: test_let_statement - s identifier token_literal not equal to "<<name<<std::endl;
            return false;
        }
        if(ls->ident().value() != name) {
            std::cout<<"fail: test_let_statement - s identifier name not equal to "<<name<<std::endl;
            return false;
        }
        return true;
    };
    
    for(int i=0;i<3;i++){
        ast::statement* stmt = program->statements()[i].get();

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
    
    if(program->statements().size() != 3) {
        std::cout<<"fail: test_return_statement - program statements not equal to 3, got "
            <<program->statements().size()<<std::endl;
        exit(EXIT_FAILURE);
    }
    
    for(int i=0;i<3;i++){
        ast::statement* s = program->statements()[i].get();
        ast::return_statement* rs = dynamic_cast<ast::return_statement*>(s);
        if(rs == nullptr){
            std::cout
                <<"fail: test_return_statement - statement "
                <<i<<" is not a return statement. got "
                <<s->token_literal()
                <<std::endl;
            exit(EXIT_FAILURE);
        }

        if(rs->token_literal() != "return") {
            std::cout
                <<"fail: test_return_statement - statement "
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
    
    if(program->statements().size() != 1) {
        std::cout<<"fail: test_ident_expr - statements.size() not 1, got "
            <<program->statements().size()<<std::endl;
        exit(EXIT_FAILURE);
    }
    
    ast::statement* s = program->statements()[0].get();
    ast::expression_statement* es = dynamic_cast<ast::expression_statement*>(s);

    if(es == nullptr) {
        std::cout<<"fail: test_ident_expr - statement not expression statement."<<std::endl;
        exit(EXIT_FAILURE);
    }

    ast::expression* e = es->expr();
    test_identifier(e, "foobar");

    std::cout<<"3 - ok: parse expression statement with identifier."<<std::endl;
}

void test_integer_literal_expression() {
    const char* input = "5;";

    lexer::lexer l(input);
    parser p(l);
    ast::program* program = p.parse_program();
    check_parser_errors(p);
    
    if(program->statements().size() != 1) {
        std::cout<<"fail: test_int_literal_expr - statements.size() not 1, got "
            <<program->statements().size()<<std::endl;
        exit(EXIT_FAILURE);
    }
    
    ast::statement* s = program->statements()[0].get();
    ast::expression_statement* es = dynamic_cast<ast::expression_statement*>(s);

    if(es == nullptr) {
        std::cout<<"fail: test_int_literal_expr - statement not expression statement."<<std::endl;
        exit(EXIT_FAILURE);
    }

    ast::expression* e = es->expr();
    test_integer_literal(e, 5);

    std::cout<<"4 - ok: parse integer literal rvalue."<<std::endl;
}

void test_parse_prefix_expression() {
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

    for(int i=0;i<prefix_test.size();i++) {
        test_case tc = prefix_test[i];
        lexer::lexer l(tc.input);
        parser p(l);
        ast::program* program = p.parse_program();
        check_parser_errors(p);

        if(program->statements().size() != 1) {
            std::cout<<"fail: test_parse_prefix - statements.size() not 1, got "
                <<program->statements().size()<<std::endl;
            exit(EXIT_FAILURE);
        }

        ast::statement* s = program->statements()[0].get();
        ast::expression_statement* es = dynamic_cast<ast::expression_statement*>(s);

        if(es == nullptr) {
            std::cout<<"fail: test_parse_prefix - statement not expression statement."<<std::endl;
            exit(EXIT_FAILURE);
        }

        ast::expression* e = es->expr();
        ast::prefix_expression* pf = dynamic_cast<ast::prefix_expression*>(e);

        if(pf == nullptr) {
            std::cout<<"fail: test_parse_prefix - expression not a prefix expression."<<std::endl;
            exit(EXIT_FAILURE);
        }

        if(pf->op() != tc.op) {
            std::cout<<"fail: test_parse_prefix - op not "<<tc.op<<", got "<<pf->op()<<std::endl;
            exit(EXIT_FAILURE);
        } 

        test_literal_expression(pf->expr(), tc.int_value);
    }
    std::cout<<"5 - ok: parse prefix with ints."<<std::endl;
}

void test_parse_infix_expression() {
    struct test_case {
        const char*     input;
        const char*     op;
        std::int64_t    left_value;
        std::int64_t    right_value;
        test_case(const char* input, std::int64_t lv, const char* op, std::int64_t rv) :
            input(input), left_value(lv), op(op), right_value(rv) {};
    };
    std::vector<test_case> infix_test {
        {"5 + 5;", 5, "+", 5},
        {"5 - 5;", 5, "-", 5},
        {"5 * 5;", 5, "*", 5},
        {"5 / 5;", 5, "/", 5},
        {"5 > 5;", 5, ">", 5},
        {"5 < 5;", 5, "<", 5},
        {"5 == 5;", 5, "==", 5},
        {"5 != 5;", 5, "!=", 5},
    };

    for(int i=0;i<infix_test.size();i++) {
        test_case tc = infix_test[i];
        lexer::lexer l(tc.input);
        parser p(l);
        ast::program* program = p.parse_program();
        check_parser_errors(p);

        if(program->statements().size() != 1) {
            std::cout<<"fail: test_parse_prefix - statements.size() not 1, got "
                <<program->statements().size()<<std::endl;
            exit(EXIT_FAILURE);
        }

        ast::statement* s = program->statements()[0].get();
        ast::expression_statement* es = dynamic_cast<ast::expression_statement*>(s);

        if(es == nullptr) {
            std::cout<<"fail: test_parse_prefix - statement not expression statement."<<std::endl;
            exit(EXIT_FAILURE);
        }

        ast::expression* e = es->expr();
        test_infix_expression(e, tc.left_value, tc.op, tc.right_value);
    }

    std::cout<<"6 - ok: parse infix expr with ints."<<std::endl;
}

void test_parse_operator_precedence() {
    struct test_case {
        const char* input;
        std::string expected;
    };
    std::vector<test_case> infix_test {
        {"-a * b", "((-a) * b)\n"},
        {"!-a", "(!(-a))\n"},
        {"a + b + c", "((a + b) + c)\n"},
        {"a + b - c", "((a + b) - c)\n"},
        {"a * b * c", "((a * b) * c)\n"},
        {"a * b / c", "((a * b) / c)\n"},
        {"a + b / c", "(a + (b / c))\n"},
        {"a + b * c + d / e - f", "(((a + (b * c)) + (d / e)) - f)\n"},
        {"3 + 4; -5 * 5", "(3 + 4)\n((-5) * 5)\n"},
        {"5 > 4 == 3 < 4", "((5 > 4) == (3 < 4))\n"},
        {"5 < 4 != 3 > 4", "((5 < 4) != (3 > 4))\n"},
        {"3 + 4 * 5 == 3 * 1 + 4 * 5", "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))\n"}
    };

    for(int i=0;i<infix_test.size();i++) {
        test_case tc = infix_test[i];
        lexer::lexer l(tc.input);
        parser p(l);
        ast::program* program = p.parse_program();
        check_parser_errors(p);

        std::string actual = program->to_string();
        if(actual != tc.expected) {
            std::cout<<"fail: test_parse_op_precedence - expected "<<tc.expected<<", got "<<actual<<std::endl;
            exit(EXIT_FAILURE);
        }
    }
    std::cout<<"7 - ok: parse general prefix + infix exprs w precedence"<<std::endl;
}

void test_boolean_expression() {
    const char* input = "true;";

    lexer::lexer l(input);
    parser p(l);
    ast::program* program = p.parse_program();
    check_parser_errors(p);
 
    if(program->statements().size() != 1) {
        std::cout<<"fail: test_bool_expr - statements.size() not 1, got "
            <<program->statements().size()<<std::endl;
        exit(EXIT_FAILURE);
    }
    
    ast::statement* s = program->statements()[0].get();
    ast::expression_statement* es = dynamic_cast<ast::expression_statement*>(s);

    if(es == nullptr) {
        std::cout<<"fail: test_bool_expr - statement not expression statement."<<std::endl;
        exit(EXIT_FAILURE);
    }

    ast::expression* e = es->expr();
    ast::boolean* b = dynamic_cast<ast::boolean*>(e);

    if(b == nullptr) {
        std::cout<<"fail: test_bool_expr - expr not a boolean."<<std::endl;
        exit(EXIT_FAILURE);
    }
    if(b->value() != true) {
        std::cout<<"fail: test_identifier - ident->value() not "<<true<<", got "<<b->value()<<std::endl;
        exit(EXIT_FAILURE);
    }
    if(b->token_literal() != "true") {
        std::cout<<"fail: test_identifier - ident token_literal not true"
            <<", got "<<b->token_literal()<<std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout<<"8 - ok: parse expression statement with identifier."<<std::endl;
}


} //namespace parser


size_t parser::trace::_indent_level = 0;
bool parser::trace::_enable_trace = 0;

int main(){
    std::cout<<"Running parser_test.cpp..."<<std::endl;
    
    parser::test_let_statement();
    parser::test_return_statement();
    parser::test_identifier_expression();
    parser::test_integer_literal_expression();
    parser::test_parse_prefix_expression();
    parser::test_parse_infix_expression();
    parser::test_parse_operator_precedence();
    parser::test_boolean_expression();

    std::cout<<"parser_test.cpp: ok"<<std::endl;

    exit(EXIT_SUCCESS);
}

