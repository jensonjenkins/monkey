#include <cstdlib>
#include "../src/object.hpp"
#include "../src/parser.hpp"
#include "../src/evaluator.hpp"

namespace evaluator {

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

void test_integer_object(const object::object* obj, std::int64_t exp){
    const object::integer* res = try_cast<const object::integer*>(obj, "test_int_obj - obj not an integer.");
    assert_value(res->value(), exp, "test_int_obj - res->value()");  
}

void test_bool_object(const object::object* obj, bool exp){
    const object::boolean* res = try_cast<const object::boolean*>(obj, "test_bool_obj - obj not a boolean.");
    assert_value(res->value(), exp, "test_bool_obj - res->value()");  
}

void test_eval_integer_expression() {
    struct test_case {
        const char*     input;
        std::int64_t    expected;
        test_case(const char* i, std::int64_t e) : input(i), expected(e) {}
    };
    std::vector<test_case> tc {
        {"5", 5},
        {"10", 10}
    };
    for(int i=0;i<tc.size();i++){
        const object::object* evaluated = test_eval(tc[i].input);
        test_integer_object(evaluated, tc[i].expected);
    }
    std::cout<<"1 - ok: evaluate int literal."<<std::endl;
}

void test_bool_integer_expression() {
    struct test_case {
        const char* input;
        bool        expected;
        test_case(const char* i, bool e) : input(i), expected(e) {}
    };
    std::vector<test_case> tc {
        {"true", true},
        {"false", false}
    };
    for(int i=0;i<tc.size();i++){
        const object::object* evaluated = test_eval(tc[i].input);
        test_bool_object(evaluated, tc[i].expected);
    }
    std::cout<<"2 - ok: evaluate bool literal."<<std::endl;
}

void test_bang_operator() {
    struct test_case {
        const char* input;
        bool        expected;
        test_case(const char* i, bool e) : input(i), expected(e) {}
    };
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

} // namespace evaluator

size_t parser::trace::_indent_level = 0;
bool parser::trace::_enable_trace = 0;

int main() {
    std::cout<<"Running evaluator_test.cpp..."<<std::endl;

    evaluator::test_eval_integer_expression();
    evaluator::test_bool_integer_expression();
    evaluator::test_bang_operator();

    exit(EXIT_SUCCESS);
}


