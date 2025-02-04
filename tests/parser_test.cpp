#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
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

template <typename T>
struct let_stmt_test_case {
    T           expected_value;
    const char* input;
    const char* expected_ident;
    let_stmt_test_case(const char* input, const char* ei, T ev) 
        : input(input), expected_ident(ei), expected_value(ev) {}; 
};

template <typename To, typename From>
std::shared_ptr<To> try_cast(From from, std::string err_msg) {
    std::shared_ptr<To> casted = std::dynamic_pointer_cast<To>(from);
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

void test_integer_literal(std::shared_ptr<const ast::expression> il, std::int64_t value) {
    auto int_lit = try_cast<const ast::int_literal>(il, "test_int_lit - expr not an int lit.");
    assert_value(int_lit->value(), value, "test_int_lit - int lit values");
    assert_value(int_lit->token_literal(), std::to_string(value), "test_int_lit - int_lit token_literal");
}

void test_identifier(std::shared_ptr<const ast::expression> expr, const std::string& value) {
    auto ident = try_cast<const ast::identifier>(expr, "test_ident - expr not an ident.");
    assert_value(ident->value(), std::string_view(value), "test_ident - ident value");
    assert_value(ident->token_literal(), value, "test_ident - ident token_literal()");
}

void test_boolean_literal(std::shared_ptr<const ast::expression> expr, bool value) {
    auto b = try_cast<const ast::boolean>(expr, "test_bool_lit - expr not a bool.");
    assert_value(b->value(), value, "test_bool_lit - bool value");
    assert_value(b->token_literal() == "true", value, "test_bool_lit - bool token_literal()");
}

template <typename T>
void test_literal_expression(std::shared_ptr<const ast::expression> expr, T v) {
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
void test_infix_expression(std::shared_ptr<const ast::expression> expr, L left, std::string op, R right) {
    auto ie = try_cast<const ast::infix_expression>(expr, "test_infix_expr - expr not an infix expr.");
    test_literal_expression(ie->l_expr(), left);
    assert_value(ie->op(), op, "test_infix_expr - op");
    test_literal_expression(ie->r_expr(), right);
}

std::shared_ptr<const ast::let_statement> test_let_statement_helper(std::shared_ptr<const ast::statement> s, const char* name) {
    auto ls = try_cast<const ast::let_statement>(s, "test_let_stmt - s is not a let_statement");
    assert_value(s->token_literal(), "let", "test_let_stmt - statement token_literal");
    assert_value(ls->ident().token_literal(), name, "test_let_stmt - stmt ident token_literal");
    assert_value(ls->ident().value(), name, "test_let_stmt - stmt ident value");
    return ls;
}

template <typename T>
void test_let_statement_template(const let_stmt_test_case<T>& tc) {
    lexer::lexer l(tc.input);
    parser p(l);
    ast::program* program = p.parse_program();
    check_parser_errors(p);
    
    if(program == nullptr) {
        std::cout<<"fail: test_let_statement - parse_program() returned nullptr."<<std::endl;
    }
    assert_value(program->statements().size(), 1, "test_let_stmt - program statements");
        
    std::shared_ptr<const ast::let_statement> ls = test_let_statement_helper(program->statements()[0], tc.expected_ident); 
    std::shared_ptr<const ast::expression> val = ls->value();

    test_literal_expression(val, tc.expected_value);
    
    std::cout<<"1 - ok: parse let statements with int rvalues."<<std::endl;
}

void test_let_statement() {
    test_let_statement_template<std::int64_t>({"let x = 5;", "x", 5});
    test_let_statement_template<bool>({"let y = true;", "y", true});
    test_let_statement_template<std::string>({"let foobar = y;", "foobar", "y"});
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
    
    assert_value(program->statements().size(), 3, "test_return_stmt - program stmt size");
 
    for(int i=0;i<3;i++){
        auto rs = try_cast<const ast::return_statement>(
                program->statements()[i], "test_return_stmt - stmt is not a return stmt.");
        assert_value(rs->token_literal(), "return", "test_return_stmt - stmt token_literal");
    }

    std::cout<<"2 - ok: parse return statements with int rvalues."<<std::endl;
}

void test_identifier_expression() {
    const char* input = "foobar;";
    lexer::lexer l(input);
    parser p(l);
    ast::program* program = p.parse_program();
    check_parser_errors(p);
    
    assert_value(program->statements().size(), 1, "test_ident_expr - program stmt size"); 

    auto es = try_cast<const ast::expression_statement>(
            program->statements()[0], "test_ident_expr - s not expr stmt");

    std::shared_ptr<const ast::expression> e = es->expr();
    test_identifier(e, "foobar");

    std::cout<<"3 - ok: parse expression statement with identifier."<<std::endl;
}

void test_integer_literal_expression() {
    const char* input = "5;";
    lexer::lexer l(input);
    parser p(l);
    ast::program* program = p.parse_program();
    check_parser_errors(p);
    
    assert_value(program->statements().size(), 1, "test_int_lit_expr - program stmt size"); 
    
    auto es = try_cast<const ast::expression_statement>(
        program->statements()[0], "test_int_lit_expr - s not expr stmt.");

    std::shared_ptr<const ast::expression> e = es->expr();
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

        assert_value(program->statements().size(), 1, "test_parse_prefix_1 - program stmt size"); 

        auto es = try_cast<const ast::expression_statement>(
                program->statements()[0], "test_parse_prefix_1 - s not expr stmt.");

        std::shared_ptr<const ast::expression> e = es->expr();
        auto pf = try_cast<const ast::prefix_expression>(e, "test_parse_prefix_1 - expr not a prefix expr.");
        
        assert_value(pf->op(), tc.op, "test_parse_prefix_1 - op");
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

        assert_value(program->statements().size(), 1, "test_parse_prefix_2 - program stmt size"); 

        auto es = try_cast<const ast::expression_statement>(
                program->statements()[0], "test_parse_prefix_2 - s not expr stmt.");

        std::shared_ptr<const ast::expression> e = es->expr();
        auto pf= try_cast<const ast::prefix_expression>(e, "test_parse_prefix_2 - expr not a prefix expr.");
        
        assert_value(pf->op(), tc.op, "test_parse_prefix_2 - op");
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

        assert_value(program->statements().size(), 1, "test_parse_infix_1 - program stmt size"); 

        auto es = try_cast<const ast::expression_statement>(
                program->statements()[0], "test_parse_infix_1 - s not expr stmt.");

        std::shared_ptr<const ast::expression> e = es->expr();
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

        assert_value(program->statements().size(), 1, "test_parse_infix_2 - program stmt size"); 

        auto es = try_cast<const ast::expression_statement>(
                program->statements()[0], "test_parse_infix_2 - s not expr stmt.");

        std::shared_ptr<const ast::expression> e = es->expr();
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
        {"-a * b", "((-a) * b)"},
        {"!-a", "(!(-a))"},
        {"a + b + c", "((a + b) + c)"},
        {"a + b - c", "((a + b) - c)"},
        {"a * b * c", "((a * b) * c)"},
        {"a * b / c", "((a * b) / c)"},
        {"a + b / c", "(a + (b / c))"},
        {"a + b * c + d / e - f", "(((a + (b * c)) + (d / e)) - f)"},
        {"3 + 4; -5 * 5", "(3 + 4)((-5) * 5)"},
        {"5 > 4 == 3 < 4", "((5 > 4) == (3 < 4))"},
        {"5 < 4 != 3 > 4", "((5 < 4) != (3 > 4))"},
        {"3 + 4 * 5 == 3 * 1 + 4 * 5", "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"},
        {"true", "true"},
        {"false", "false"},
        {"3 > 5 == false", "((3 > 5) == false)"},
        {"3 < 5 == true", "((3 < 5) == true)"},
        {"1 + (2 + 3) + 4", "((1 + (2 + 3)) + 4)"},
        {"(5 + 5) * 2", "((5 + 5) * 2)"},
        {"2 / (5 + 5)", "(2 / (5 + 5))"},
        {"-(5 + 5)", "(-(5 + 5))"},
        {"!(true == true)", "(!(true == true))"},
        {"a + add(b * c) + d", "((a + add((b * c))) + d)"},
        {"add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))", "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))"},
        {"add(a + b + c * d / f + g)", "add((((a + b) + ((c * d) / f)) + g))"},
        {"a * [1, 2, 3, 4][b * c] * d", "((a * ([1, 2, 3, 4][(b * c)])) * d)"},
        {"add(a * b[2], b[1], 2 * [1, 2][1])", "add((a * (b[2])), (b[1]), (2 * ([1, 2][1])))"},
    };

    for(int i=0;i<infix_test.size();i++) {
        test_case tc = infix_test[i];
        lexer::lexer l(tc.input);
        parser p(l);
        ast::program* program = p.parse_program();
        check_parser_errors(p);

        std::string actual = program->to_string();
        assert_value(actual, tc.expected, "test_parse_op_precedence - to_string");
    }
    std::cout<<"7 - ok: parse general prefix + infix exprs w precedence"<<std::endl;
}

void test_boolean_expression() {
    const char* input = "true;";
    lexer::lexer l(input);
    parser p(l);
    ast::program* program = p.parse_program();
    check_parser_errors(p);
 
    assert_value(program->statements().size(), 1, "test_bool_expr - program stmt size"); 
    
    auto es = try_cast<const ast::expression_statement>(program->statements()[0], "test_bool_expr - s not expr stmt.");

    std::shared_ptr<const ast::expression> e = es->expr();
    auto b = try_cast<const ast::boolean>(e, "test_bool_expr - expr not a boolean.");
    assert_value(b->value(), true, "test_ident - bool value");
    assert_value(b->token_literal(), "true", "test_ident - bool token_literal");

    std::cout<<"8 - ok: parse boolean expr."<<std::endl;
}

void test_if_expression() {
    std::string lv = "x", op = "<", rv = "y";
    const char* input = "if (x < y) { x }";
    lexer::lexer l(input);
    parser p(l);
    ast::program* program = p.parse_program();
    check_parser_errors(p);
    
    assert_value(program->statements().size(), 1, "test_if_expr - program stmt size"); 
    
    auto es = try_cast<const ast::expression_statement>(program->statements()[0], "test_if_expr - s not expr stmt.");

    std::shared_ptr<const ast::expression> e = es->expr();
    auto ie = try_cast<const ast::if_expression>(e, "test_if_expression - e not an if expr");
    test_infix_expression(ie->condition(), lv, op, rv);

    assert_value(ie->consequence()->statements().size(), 1, "test_if_expr - consequence stmt size");
    auto c = try_cast<const ast::expression_statement>(
            ie->consequence()->statements()[0], "test_if_expression - consequence is not an expr stmt");
    test_identifier(c->expr(), "x");

    assert_value(ie->alternative(), nullptr, "test_if_expr - alternative");

    std::cout<<"9.1 - ok: parse if expression."<<std::endl;
}

void test_if_else_expression() {
    std::string lv = "x", op = "<", rv = "y";
    const char* input = "if (x < y) { x } else { y }";
    lexer::lexer l(input);
    parser p(l);
    ast::program* program = p.parse_program();
    check_parser_errors(p);
    
    assert_value(program->statements().size(), 1, "test_if_expr - program stmt size"); 
    
    auto es = try_cast<const ast::expression_statement>(program->statements()[0], "test_if_expr - s not expr stmt.");

    std::shared_ptr<const ast::expression> e = es->expr();
    auto ie = try_cast<const ast::if_expression>(e, "test_if_expression - e not an if expr");
    test_infix_expression(ie->condition(), lv, op, rv);

    assert_value(ie->consequence()->statements().size(), 1, "test_if_expr - consequence stmt size");
    auto c = try_cast<const ast::expression_statement>(
            ie->consequence()->statements()[0], "test_if_expression - consequence is not an expr stmt");
    test_identifier(c->expr(), "x");
    
    assert_value(ie->alternative()->statements().size(), 1, "test_if_expr - alternative statements size");
    auto a = try_cast<const ast::expression_statement>(
            ie->alternative()->statements()[0], "test_if_expr - alternative is not an expr stmt.");
    test_identifier(a->expr(), "y");


    std::cout<<"9.2 - ok: parse if else expression."<<std::endl;
}

void test_parse_function_literal() {
    const char* input = "fn(x, y) { x + y; }";
    std::string lv = "x";
    std::string op = "+";
    std::string rv = "y";

    lexer::lexer l(input);
    parser p(l);
    ast::program* program = p.parse_program();
    check_parser_errors(p);
    
    assert_value(program->statements().size(), 1, "test_fn_lit - statements.size() dont match.");
    
    auto es = try_cast<const ast::expression_statement>(
            program->statements()[0], "test_fn_lit - statement not an expr statment.");

    std::shared_ptr<const ast::expression> e = es->expr();
    auto fl = try_cast<const ast::function_literal>(e, "test_function_literal - expr not a function literal.");
    assert_value(fl->parameters().size(), 2, "test_fn_lit - parameters.size() dont match.");

    test_literal_expression(fl->parameters()[0], lv);
    test_literal_expression(fl->parameters()[1], rv);

    assert_value(fl->body()->statements().size(), 1, "test_fn_lit - body statements size() dont match.");

    auto stmt = try_cast<const ast::expression_statement>(fl->body()->statements()[0],
            "test_function_literal - body statement not expression statement.");

    test_infix_expression(stmt->expr(), lv, op, rv);

    std::cout<<"10 - ok: parse function literal."<<std::endl;
}

void test_parse_function_parameter() {
    struct test_case {
        const char* input;
        std::vector<std::string> expected;
        test_case(const char* i, std::vector<std::string> e) : input(i), expected(e) {}
    };
    std::vector<test_case> fn_param_test {
        {"fn() {}", std::vector<std::string>{}},
        {"fn(x) {}", std::vector<std::string>{"x"}},
        {"fn(x, y, z) {}", std::vector<std::string>{"x", "y", "z"}},
    };
    for(int i=0;i<fn_param_test.size();i++) {
        test_case tc = fn_param_test[i];
        lexer::lexer l(tc.input);
        parser p(l);
        ast::program* program = p.parse_program();
        check_parser_errors(p);

        auto stmt = try_cast<const ast::expression_statement>(
                program->statements()[0], "test_parse_fn_param - stmt not an expr stmt.");
        auto fn = try_cast<const ast::function_literal>(stmt->expr(), "test_parse_fn_param - expr stmt not a fn literal.");

        assert_value(fn->parameters().size(), tc.expected.size(), "test_parse_fn_param - parameter count");
        for(int j=0;j<tc.expected.size();j++) {
            test_literal_expression(fn->parameters()[j], tc.expected[j]);
        }
    }
    std::cout<<"11 - ok: parse function parameters."<<std::endl;
}

void test_parse_call_expression() {
    const char* input = "add(1, 2 * 3, 4 + 5);";
    lexer::lexer l(input);
    parser p(l);
    ast::program* program = p.parse_program();
    check_parser_errors(p);

    assert_value(program->statements().size(), 1, "test_parse_call_expr - program statements size");

    auto e = try_cast<const ast::expression_statement>(
            program->statements()[0], "test_parse_call_expr - statement not an expr stmt.");
    auto call = try_cast<const ast::call_expression>(e->expr(), "test_parse_call_expr - expr stmt not an call expr.");

    test_identifier(call->function(), "add");
    assert_value(call->arguments().size(), 3, "test_parse_call_expr - number of call expr arguments");
    
    test_literal_expression(call->arguments()[0], 1);
    test_infix_expression(call->arguments()[1], 2, "*", 3);
    test_infix_expression(call->arguments()[2], 4, "+", 5);

    std::cout<<"12 - ok: parse function call expression."<<std::endl;
}

void test_string_literal_expression() {
    const char* input = R"(
        "hello world";
    )";
    lexer::lexer l(input);
    parser p(l);
    ast::program* program = p.parse_program();
    check_parser_errors(p);

    assert_value(program->statements().size(), 1, "test_str_lit_expr - program statements size");
    auto e = try_cast<const ast::expression_statement>(
            program->statements()[0], "test_str_lit_expr - statement not an expr stmt.");
    auto lit = try_cast<const ast::string_literal>(e->expr(), "test_str_lit_expr - statement not a string literal.");
    assert_value(lit->value(), "hello world", "test_str_lit_expr - string literal value");

    std::cout<<"13 - ok: parse string literals."<<std::endl;
}

void test_parse_array_literal() {
    const char* input = "[1, 2 * 2, 3 + 3]";
    lexer::lexer l(input);
    parser p(l);
    ast::program* program = p.parse_program();
    check_parser_errors(p);

    assert_value(program->statements().size(), 1, "test_parse_array_lit - program statements size");
    auto e = try_cast<const ast::expression_statement>(
            program->statements()[0], "test_parse_array_lit - statement not an expr stmt.");
    auto array = try_cast<const ast::array_literal>(e->expr(), "test_parse_array_lit - statement not an array literal.");
    assert_value(array->elements().size(), 3, "test_parse_array_lit - array literal size");

    test_integer_literal(array->elements()[0], 1);
    test_infix_expression(array->elements()[1], 2, "*", 2);
    test_infix_expression(array->elements()[2], 3, "+", 3);

    std::cout<<"14 - ok: parse array literal."<<std::endl;
}

void test_parse_index_expression() {
    const char* input = "my_array[1 + 1]";
    lexer::lexer l(input);
    parser p(l);
    ast::program* program = p.parse_program();
    check_parser_errors(p);

    assert_value(program->statements().size(), 1, "test_parse_index_expr - program statements size");
    auto e = try_cast<const ast::expression_statement>(
            program->statements()[0], "test_parse_index_expr - statement not an expr stmt.");
    auto index_expr = try_cast<const ast::index_expression>(e->expr(), "test_parse_index_expr - statement not an index expr.");

    test_identifier(index_expr->left(), "my_array");
    test_infix_expression(index_expr->index(), 1, "+", 1);

    std::cout<<"15 - ok: parse index expression."<<std::endl;
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
    parser::test_parse_function_literal();
    parser::test_parse_function_parameter();
    parser::test_parse_call_expression();
    parser::test_string_literal_expression();
    parser::test_parse_array_literal();
    parser::test_parse_index_expression();

    std::cout<<"parser_test.cpp: ok"<<std::endl;

    exit(EXIT_SUCCESS);
}

