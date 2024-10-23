#include <cstdlib>
#include "../src/object.hpp"
#include "../src/parser.hpp"
#include "../src/evaluator.hpp"

namespace evaluator {

template <typename T>
struct test_case_base {
    T           expected;
    const char* input;
    test_case_base(const char* i, T e) : input(i), expected(e) {}
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

const object::object* test_eval(const char* input) {
    lexer::lexer l(input);
    parser::parser p(l);
    ast::program* program = p.parse_program();

    return evaluator::eval(program);
}

void test_integer_object(const object::object* obj, std::int64_t exp) {
    const object::integer* res = try_cast<const object::integer*>(obj, "test_int_obj - obj not an integer.");
    assert_value(res->value(), exp, "test_int_obj - res->value()");  
}

void test_bool_object(const object::object* obj, bool exp) {
    const object::boolean* res = try_cast<const object::boolean*>(obj, "test_bool_obj - obj not a boolean.");
    assert_value(res->value(), exp, "test_bool_obj - res->value()");  
}

void test_null_object(const object::object* obj) {
    const object::null* res = try_cast<const object::null*>(obj, "test_null_obj - obj not null.");
}

void test_eval_integer_expression() {
    using test_case = test_case_base<std::int64_t>;
    std::vector<test_case> tc {
        {"5", 5},
        {"10", 10},
        {"-5", -5},
        {"-10", -10},
        {"5 + 5", 10},
        {"5 + 5 + 5 + 5 - 10", 10},
        {"2 * 2 * 2 * 2 * 2", 32},
        {"-50 + 100 + -50", 0},
        {"5 * 2 + 10", 20},
        {"5 + 2 * 10", 25},
        {"20 + 2 * -10", 0},
        {"50 / 2 * 2 + 10", 60},
        {"2 * (5 + 10)", 30},
        {"3 * 3 * 3 + 10", 37},
        {"3 * (3 * 3) + 10", 37},
        {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
    };
    for(int i=0;i<tc.size();i++){
        const object::object* evaluated = test_eval(tc[i].input);
        test_integer_object(evaluated, tc[i].expected);
    }
    std::cout<<"1 - ok: evaluate int literal."<<std::endl;
}

void test_eval_bool_expression() {
    using test_case = test_case_base<bool>;
    std::vector<test_case> tc {
        {"true", true},
        {"false", false},
        {"1 < 2", true},
        {"1 > 2", false},
        {"1 < 1", false},
        {"1 > 1", false},
        {"1 == 2", false},
        {"1 != 2", true},
        {"1 == 1", true},
        {"1 != 1", false},
        {"true == true", true},
        {"false == false", true},
        {"false == true", false},
        {"true == false", false},
        {"false != true", true},
        {"true != false", true},
        {"(1 < 2) != false", true},
        {"(1 > 2) == false", true},
        {"(1 > 2) == true", false},
        {"(1 < 2) == false", false},
    };
    for(int i=0;i<tc.size();i++){
        const object::object* evaluated = test_eval(tc[i].input);
        test_bool_object(evaluated, tc[i].expected);
    }
    std::cout<<"2 - ok: evaluate bool literal."<<std::endl;
}

void test_eval_bang_operator() {
    using test_case = test_case_base<bool>;
    std::vector<test_case> tc {
        {"!true", false},
        {"!false", true},
        {"!5", false},
        {"!!true", true},
        {"!!false", false},
        {"!!5", true},
    };
    for(int i=0;i<tc.size();i++){
        const object::object* evaluated = test_eval(tc[i].input);
        test_bool_object(evaluated, tc[i].expected);
    }
    std::cout<<"3 - ok: evaluate bang prefix operator."<<std::endl;
}

void test_if_else_expression() {
    using test_case = test_case_base<std::optional<std::int64_t>>;
    std::vector<test_case> tc {
        {"if (true) { 10 }", 10},
        {"if (false) { 10 }", std::nullopt},
        {"if (1) { 10 }", 10},
        {"if (1 < 2) { 10 }", 10},
        {"if (1 > 2) { 10 }", std::nullopt},
        {"if (1 < 2) { 10 } else { 20 }", 10},
        {"if (1 > 2) { 10 } else { 20 }", 20},
    };
    for(int i=0;i<tc.size();i++){
        const object::object* evaluated = test_eval(tc[i].input);
        if(tc[i].expected){
            test_integer_object(evaluated, tc[i].expected.value());
        } else {
            test_null_object(evaluated);
        }
    }
    std::cout<<"4 - ok: evaluate if else expr."<<std::endl;
}

void test_return_statements() {
    using test_case = test_case_base<std::int64_t>;
    std::vector<test_case> tc {
        {"return 10;", 10},
        {"return 10; 9;", 10},
        {"return 2 * 55; 9;", 110},
        {"9; return 3 * 5; 9;", 15},
        {R"(
            if(10 > 1) {
                if(20 > 2){
                    return 20;
                }
                return 1;
            }
        )", 20}
    };
    for(int i=0;i<tc.size();i++){
        const object::object* evaluated = test_eval(tc[i].input);
        test_integer_object(evaluated, tc[i].expected);
    }
    std::cout<<"5 - ok: return statements."<<std::endl;
}

} // namespace evaluator

size_t parser::trace::_indent_level = 0;
bool parser::trace::_enable_trace = 0;

int main() {
    std::cout<<"Running evaluator_test.cpp..."<<std::endl;

    evaluator::test_eval_integer_expression();
    evaluator::test_eval_bool_expression();
    evaluator::test_eval_bang_operator();
    evaluator::test_if_else_expression();
    evaluator::test_return_statements();

    exit(EXIT_SUCCESS);
}


