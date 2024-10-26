#include "../src/evaluator.hpp"
#include "../src/object.hpp"
#include "../src/parser.hpp"
#include <cstdlib>

namespace evaluator {

template <typename T> struct test_case_base {
  T expected;
  const char *input;
  test_case_base(const char *i, T e) : input(i), expected(e) {}
};

template <typename To, typename From>
To try_cast(From from, std::string err_msg) {
  To casted = dynamic_cast<To>(from);
  if (casted == nullptr) {
    std::cout << "fail: " << err_msg << std::endl;
    exit(EXIT_FAILURE);
  }
  return casted;
}

template <typename T, typename V>
void assert_value(const T &actual, const V &expected, std::string err_msg) {
  if (actual != expected) {
    std::cout << "fail: " << err_msg << " does not match. expected " << expected
              << " , got " << actual << std::endl;
    exit(EXIT_FAILURE);
  }
}

const object::object *test_eval(const char *input) {
  lexer::lexer l(input);
  parser::parser p(l);
  ast::program *program = p.parse_program();
  object::scope *scope = new object::scope();

  return evaluator::eval(program, scope);
}

void test_integer_object(const object::object *obj, std::int64_t exp) {
  const object::integer *res = try_cast<const object::integer *>(
      obj, "test_int_obj - obj not an integer.");
  assert_value(res->value(), exp, "test_int_obj - res->value()");
}

void test_bool_object(const object::object *obj, bool exp) {
  const object::boolean *res = try_cast<const object::boolean *>(
      obj, "test_bool_obj - obj not a boolean.");
  assert_value(res->value(), exp, "test_bool_obj - res->value()");
}

void test_null_object(const object::object *obj) {
  const object::null *res =
      try_cast<const object::null *>(obj, "test_null_obj - obj not null.");
}

void test_eval_integer_expression() {
  using test_case = test_case_base<std::int64_t>;
  std::vector<test_case> tc{
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
  for (int i = 0; i < tc.size(); i++) {
    const object::object *evaluated = test_eval(tc[i].input);
    test_integer_object(evaluated, tc[i].expected);
  }
  std::cout << "1 - ok: evaluate int literal." << std::endl;
}

void test_eval_bool_expression() {
  using test_case = test_case_base<bool>;
  std::vector<test_case> tc{
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
  for (int i = 0; i < tc.size(); i++) {
    const object::object *evaluated = test_eval(tc[i].input);
    test_bool_object(evaluated, tc[i].expected);
  }
  std::cout << "2 - ok: evaluate bool literal." << std::endl;
}

void test_eval_bang_operator() {
  using test_case = test_case_base<bool>;
  std::vector<test_case> tc{
      {"!true", false}, {"!false", true},   {"!5", false},
      {"!!true", true}, {"!!false", false}, {"!!5", true},
  };
  for (int i = 0; i < tc.size(); i++) {
    const object::object *evaluated = test_eval(tc[i].input);
    test_bool_object(evaluated, tc[i].expected);
  }
  std::cout << "3 - ok: evaluate bang prefix operator." << std::endl;
}

void test_if_else_expression() {
  using test_case = test_case_base<std::optional<std::int64_t>>;
  std::vector<test_case> tc{
      {"if (true) { 10 }", 10},
      {"if (false) { 10 }", std::nullopt},
      {"if (1) { 10 }", 10},
      {"if (1 < 2) { 10 }", 10},
      {"if (1 > 2) { 10 }", std::nullopt},
      {"if (1 < 2) { 10 } else { 20 }", 10},
      {"if (1 > 2) { 10 } else { 20 }", 20},
  };
  for (int i = 0; i < tc.size(); i++) {
    const object::object *evaluated = test_eval(tc[i].input);
    if (tc[i].expected) {
      test_integer_object(evaluated, tc[i].expected.value());
    } else {
      test_null_object(evaluated);
    }
  }
  std::cout << "4 - ok: evaluate if else expr." << std::endl;
}

void test_return_statements() {
  using test_case = test_case_base<std::int64_t>;
  std::vector<test_case> tc{{"return 10;", 10},
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
        )",
                             20}};
  for (int i = 0; i < tc.size(); i++) {
    const object::object *evaluated = test_eval(tc[i].input);
    test_integer_object(evaluated, tc[i].expected);
  }
  std::cout << "5 - ok: return statements." << std::endl;
}

void test_error_handling() {
  using test_case = test_case_base<std::string>;
  std::vector<test_case> tc{
      {"5 + true;", "type mismatch: INTEGER + BOOLEAN"},
      {"5 + true; 5;", "type mismatch: INTEGER + BOOLEAN"},
      {"-true", "unknown operator: -BOOLEAN"},
      {"true + true;", "unknown operator: BOOLEAN + BOOLEAN"},
      {"5; true + true; 5", "unknown operator: BOOLEAN + BOOLEAN"},
      {"if (10 > 1) { true + true; }", "unknown operator: BOOLEAN + BOOLEAN"},
      {R"(
            if(10 > 1) {
                if(20 > 2){
                    return true + false;
                }
                return 1;
            }
        )",
       "unknown operator: BOOLEAN + BOOLEAN"},
      {"foobar", "identifier not found: foobar"},
  };
  for (int i = 0; i < tc.size(); i++) {
    object::object *evaluated =
        const_cast<object::object *>(test_eval(tc[i].input));
    object::error *eo = try_cast<object::error *>(
        evaluated, "test_error_handling - not an error obj.");
    assert_value(eo->inspect(), tc[i].expected,
                 "test_error_handling - error message");
  }
  std::cout << "6 - ok: evaluate error handling." << std::endl;
}

void test_eval_let_statement() {
  using test_case = test_case_base<std::int64_t>;
  std::vector<test_case> tc{
      {"let a = 5; a;", 5},
      {"let a = 10 * 5; a;", 50},
      {"let a = 5; let b = a; b;", 5},
      {"let a = 5; let b = a; let c = a + b + 5; c;", 15}};
  for (int i = 0; i < tc.size(); i++) {
    test_integer_object(test_eval(tc[i].input), tc[i].expected);
  }
  std::cout << "7 - ok: evaluate let statements." << std::endl;
}

void test_function_object() {
  const char *input = "fn(x) { x + 2; };";
  object::object *evaluated = const_cast<object::object *>(test_eval(input));
  object::function *fn = try_cast<object::function *>(
      evaluated, "test_function_object - not a fn obj.");
  assert_value(fn->parameters().size(), 1,
               "test_function_object - parameter count");
  assert_value(fn->parameters()[0]->to_string(), "x",
               "test_function_object - parameter");
  assert_value(fn->body()->to_string(), "(x + 2)",
               "test_function_object - body");

  std::cout << "8 - ok: evaluate fn object." << std::endl;
}

void test_eval_function() {
  using test_case = test_case_base<std::int64_t>;
  std::vector<test_case> tc{
      {"let identity = fn(x) { x; }; identity(6);", 6},
      {"let identity = fn(x) { return x; }; identity(4);", 4},
      {"let double = fn(x) { x * 2; }; double(6);", 12},
      {"let add = fn(x, y) { x + y; }; add(112, 10);", 122},
      {R"(
        let add = fn(x, y) { x + y; }; 
        let sub = fn(x, y) { x - y; }; 
        add(10, sub(9,  8));
        )",
       11},
      {"fn(x) { x; }(5)", 5}};
  for (int i = 0; i < tc.size(); i++) {
    test_integer_object(test_eval(tc[i].input), tc[i].expected);
  }

  std::cout << "9 - ok: evaluate fn expressions." << std::endl;
}

void test_recursion() {
  const char *input = R"(
    let counter = fn(x){
        if(x > 1) {
            return 123;
        } else{
            counter(x + 1);
        }
    };
    counter(0);
    )";
    test_integer_object(test_eval(input), 123);
  std::cout << "10 - ok: evaluate recursion." << std::endl;
}

} // namespace evaluator

size_t parser::trace::_indent_level = 0;
bool parser::trace::_enable_trace = 0;

int main() {
  std::cout << "Running evaluator_test.cpp..." << std::endl;

  evaluator::test_eval_integer_expression();
  evaluator::test_eval_bool_expression();
  evaluator::test_eval_bang_operator();
  evaluator::test_if_else_expression();
  evaluator::test_return_statements();
  evaluator::test_error_handling();
  evaluator::test_eval_let_statement();
  evaluator::test_function_object();
  evaluator::test_eval_function();
  evaluator::test_recursion();

  exit(EXIT_SUCCESS);
}
