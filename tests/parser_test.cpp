#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <type_traits>

#include "../src/parser.hpp"

namespace parser {

template <typename L, typename R>
struct parse_infix_test_case {
    L               left_value;
    R               right_value;
    const char*     input;
    const char*     op;
    parse_infix_test_case(const char* input, L lv, const char* op, R rv) :
        input(input), left_value(lv), op(op), right_value(rv) {};
};

template <typename T>
struct parse_prefix_test_case{
    T               value;
    const char*     input;
    const char*     op;
    parse_prefix_test_case(const char* input, const char* op, T v) : input(input), op(op), value(v) {};
};

template <typename To, typename From>
To try_cast(From from, std::string err_msg) {
    To casted = dynamic_cast<To>(from);
    if(casted == nullptr){
        std::cout<<"fail: "<<err_msg<<std::endl;
        exit(EXIT_FAILURE);
    }
    return casted;
}

template <typename T, typename V>
void assert_value(const T& actual, const V& expected, std::string err_msg){
    if(actual != expected) {
        std::cout<<"fail: "<<err_msg<<" does not match. expected "<<expected<<" , got "<<actual<<std::endl;
        exit(EXIT_FAILURE);
    }
}

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
    ast::int_literal* int_lit = try_cast<ast::int_literal*>(il, "test_int_lit - expr not an int lit.");
    assert_value(int_lit->value(), value, "test_int_lit - int lit values");
    assert_value(int_lit->token_literal(), std::to_string(value), "test_int_lit - int_lit token_literal");
}

void test_identifier(ast::expression* expr, const std::string& value) {
    ast::identifier* ident = try_cast<ast::identifier*>(expr, "test_ident - expr not an ident.");
    assert_value(ident->value(), std::string_view(value), "test_ident - ident value");
    assert_value(ident->token_literal(), value, "test_ident - ident token_literal()");
}

void test_boolean_literal(ast::expression* expr, bool value) {
    ast::boolean* b = try_cast<ast::boolean*>(expr, "test_bool_lit - expr not a bool.");
    assert_value(b->value(), value, "test_bool_lit - bool value");
    assert_value(b->token_literal() == "true", value, "test_bool_lit - bool token_literal()");
}

template <typename T>
void test_literal_expression(ast::expression* expr, T v) {
    if constexpr(std::is_same_v<T, int> || std::is_same_v<T, std::int64_t>) {
        return test_integer_literal(expr, v);
    } else if constexpr (std::is_same_v<T, std::string>) {
        return test_identifier(expr, std::string(v));
    } else if constexpr (std::is_same_v<T, bool>) {
        return test_boolean_literal(expr, v);
    }
    std::cout<<"fail: test_literal_expression - type of expression not handled: "<<typeid(T).name()<<std::endl;
    exit(EXIT_FAILURE);
}

template <typename L, typename R>
void test_infix_expression(ast::expression* expr, L left, std::string op, R right) {
    ast::infix_expression* ie = try_cast<ast::infix_expression*>(expr, "test_infix_expr - expr not an infix expr.");
    test_literal_expression(ie->l_expr(), left);
    assert_value(ie->op(), op, "test_infix_expr - op");
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

void test_parse_prefix_expression_1() {
    using test_case = parse_prefix_test_case<std::int64_t>;
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

        test_literal_expression(pf->expr(), tc.value);
    }
    std::cout<<"5.1 - ok: parse prefix with ints."<<std::endl;
}

void test_parse_prefix_expression_2() {
    using test_case = parse_prefix_test_case<bool>;
    std::vector<test_case> prefix_test {
        {"!true;", "!", true},
        {"!false;", "!", false}
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

        test_literal_expression(pf->expr(), tc.value);
    }
    std::cout<<"5.2 - ok: parse prefix with bools."<<std::endl;
}

void test_parse_infix_expression_1() {
    using test_case = parse_infix_test_case<std::int64_t, std::int64_t>;
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

    std::cout<<"6.1 - ok: parse infix expr with ints."<<std::endl;
}

void test_parse_infix_expression_2() {
    using test_case = parse_infix_test_case<bool, bool>;
    std::vector<test_case> infix_test {
        {"true == true;", true, "==", true},
        {"true != false;", true, "!=", false},
        {"false == false;", false, "==", false},
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

    std::cout<<"6.2 - ok: parse infix expr with bools."<<std::endl;
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
        {"3 + 4 * 5 == 3 * 1 + 4 * 5", "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))\n"},
        {"true", "true\n"},
        {"false", "false\n"},
        {"3 > 5 == false", "((3 > 5) == false)\n"},
        {"3 < 5 == true", "((3 < 5) == true)\n"},
        {"1 + (2 + 3) + 4", "((1 + (2 + 3)) + 4)\n"},
        {"(5 + 5) * 2", "((5 + 5) * 2)\n"},
        {"2 / (5 + 5)", "(2 / (5 + 5))\n"},
        {"-(5 + 5)", "(-(5 + 5))\n"},
        {"!(true == true)", "(!(true == true))\n"},
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

    std::cout<<"8 - ok: parse boolean expr."<<std::endl;
}

void test_if_expression() {
    const char* input = "if (x < y) { x }";
    lexer::lexer l(input);
    parser p(l);
    ast::program* program = p.parse_program();
    check_parser_errors(p);
    
    if(program->statements().size() != 1) {
        std::cout<<"fail: test_if_expression - statements.size() not 1, got "
            <<program->statements().size()<<std::endl;
        exit(EXIT_FAILURE);
    }
    
    ast::statement* s = program->statements()[0].get();
    ast::expression_statement* es = dynamic_cast<ast::expression_statement*>(s);

    if(es == nullptr) {
        std::cout<<"fail: test_if_expression - statement not expression statement."<<std::endl;
        exit(EXIT_FAILURE);
    }

    ast::expression* e = es->expr();
    ast::if_expression* ie = dynamic_cast<ast::if_expression*>(e);

    if(ie == nullptr) {
        std::cout<<"fail: test_if_expression - expr not an if expression."<<std::endl;
        exit(EXIT_FAILURE);
    }
    std::string lv = "x";
    std::string op = "<";
    std::string rv = "y";
    test_infix_expression(ie->condition(), lv, op, rv);

    if(ie->consequence()->statements().size() != 1) {
        std::cout<<"fail: test_if_expression - consequence statements size() not 1, got "
            <<ie->consequence()->statements().size()<<std::endl;
        exit(EXIT_FAILURE);
    }

    ast::expression_statement* c = 
        dynamic_cast<ast::expression_statement*>(ie->consequence()->statements()[0].get());

    if(c == nullptr) {
        std::cout<<"fail: test_if_expression - consequence is not an expression statement."<<std::endl;
        exit(EXIT_FAILURE);
    }

    test_identifier(c->expr(), "x");

    if(ie->alternative() != nullptr) {
        std::cout<<"fail: test_if_expression - alternative should not exist."<<std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout<<"9.1 - ok: parse if expression."<<std::endl;
}

void test_if_else_expression() {
    const char* input = "if (x < y) { x } else { y }";
    lexer::lexer l(input);
    parser p(l);
    ast::program* program = p.parse_program();
    check_parser_errors(p);
    
    if(program->statements().size() != 1) {
        std::cout<<"fail: test_if_expression - statements.size() not 1, got "
            <<program->statements().size()<<std::endl;
        exit(EXIT_FAILURE);
    }
    
    ast::statement* s = program->statements()[0].get();
    ast::expression_statement* es = dynamic_cast<ast::expression_statement*>(s);

    if(es == nullptr) {
        std::cout<<"fail: test_if_expression - statement not expression statement."<<std::endl;
        exit(EXIT_FAILURE);
    }

    ast::expression* e = es->expr();
    ast::if_expression* ie = dynamic_cast<ast::if_expression*>(e);

    if(ie == nullptr) {
        std::cout<<"fail: test_if_expression - expr not an if expression."<<std::endl;
        exit(EXIT_FAILURE);
    }
    std::string lv = "x";
    std::string op = "<";
    std::string rv = "y";
    test_infix_expression(ie->condition(), lv, op, rv);

    if(ie->consequence()->statements().size() != 1) {
        std::cout<<"fail: test_if_expression - consequence statements size() not 1, got "
            <<ie->consequence()->statements().size()<<std::endl;
        exit(EXIT_FAILURE);
    }

    ast::expression_statement* c = 
        dynamic_cast<ast::expression_statement*>(ie->consequence()->statements()[0].get());

    if(c == nullptr) {
        std::cout<<"fail: test_if_expression - consequence is not an expression statement."<<std::endl;
        exit(EXIT_FAILURE);
    }

    test_identifier(c->expr(), "x");

    if(ie->alternative()->statements().size() != 1) {
        std::cout<<"fail: test_if_expression - alternative statements size() not 1, got "
            <<ie->alternative()->statements().size()<<std::endl;
        exit(EXIT_FAILURE);
    }

    ast::expression_statement* a = 
        dynamic_cast<ast::expression_statement*>(ie->alternative()->statements()[0].get());

    if(a == nullptr) {
        std::cout<<"fail: test_if_expression - consequence is not an expression statement."<<std::endl;
        exit(EXIT_FAILURE);
    }

    test_identifier(a->expr(), "y");


    std::cout<<"9.2 - ok: parse if else expression."<<std::endl;
}

void parse_function_literal() {
    const char* input = "fn(x, y) { x + y; }";
    std::string lv = "x";
    std::string op = "+";
    std::string rv = "y";

    lexer::lexer l(input);
    parser p(l);
    ast::program* program = p.parse_program();
    check_parser_errors(p);
    
    assert_value(program->statements().size(), 1, "test_fn_lit - statements.size() dont match.");
    
    ast::statement* s = program->statements()[0].get();
    ast::expression_statement* es = try_cast<ast::expression_statement*>(s, 
            "test_fn_lit - statement not an expr statment.");

    ast::expression* e = es->expr();
    ast::function_literal* fl = try_cast<ast::function_literal*>(e, 
            "test_function_literal - expr not a function literal.");
    assert_value(fl->parameters().size(), 2, "test_fn_lit - parameters.size() dont match.");

    test_literal_expression(fl->parameters()[0].get(), lv);
    test_literal_expression(fl->parameters()[1].get(), rv);

    assert_value(fl->body()->statements().size(), 1, "test_fn_lit - body statements size() dont match.");

    ast::expression_statement* stmt = try_cast<ast::expression_statement*>(fl->body()->statements()[0].get(), 
            "test_function_literal - body statement not expression statement.");

    test_infix_expression(stmt->expr(), lv, op, rv);
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
    parser::test_parse_prefix_expression_1();
    parser::test_parse_prefix_expression_2();
    parser::test_parse_infix_expression_1();
    parser::test_parse_infix_expression_2();
    parser::test_parse_operator_precedence();
    parser::test_boolean_expression();
    parser::test_if_expression();
    parser::test_if_else_expression();
    parser::parse_function_literal();

    std::cout<<"parser_test.cpp: ok"<<std::endl;

    exit(EXIT_SUCCESS);
}

