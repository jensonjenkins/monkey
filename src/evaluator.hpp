#pragma once

#include "builtin_fns.hpp"
#include "trace.hpp"
#include "object.hpp"
#include "ast.hpp"

namespace evaluator {

inline object::null* NULL_O = new object::null;
inline object::boolean* TRUE_O = new object::boolean(true);
inline object::boolean* FALSE_O = new object::boolean(false);

static object::object* eval(ast::node* node, object::scope* scope); // forward declaration to avoid compiler complaints

inline static bool is_error(object::object* obj) noexcept {
    if (obj != nullptr) {
        return obj->type() == object::ERROR_OBJ;
    }
    return false;
}

static object::object* eval_program (const std::vector<std::unique_ptr<ast::statement>>& stmts, object::scope* scope) {
    parser::trace t("eval_statements: " + std::to_string(stmts.size()) + " stmts.");
    object::object* result;
    
    for(const auto& stmt : stmts){
        result = eval(stmt.get(), scope);

        if(auto* rv = dynamic_cast<object::return_value*>(result)) {
            return rv->value();
        }
        if(auto* e = dynamic_cast<object::error*>(result)) {
            return e;
        }
    }

    return result;
}

static object::object* eval_bang_operator_expression(object::object* right) noexcept {
    parser::trace t("eval_bang_operator_expr: " + right->inspect());
    if(right == TRUE_O) {
        return FALSE_O;
    } else if(right == FALSE_O) {
        return TRUE_O;
    } else if(right == NULL_O) {
        return TRUE_O;
    } else {
        return FALSE_O;
    }
}

static object::object* eval_minus_prefix_operator_expression(object::object* right) noexcept {
    if(right->type() != object::INTEGER_OBJ) {
        return new object::error("unknown operator: -" + std::string(right->type()));
    }
    object::integer* r = dynamic_cast<object::integer*>(right);
    return new object::integer(-(r->value()));
}

static object::object* eval_prefix_expression(std::string_view op, object::object* right) {
    parser::trace t("eval_prefix_expression_method: " + std::string(op) + " " + right->inspect());
    if (op == "!") {
        return eval_bang_operator_expression(right);
    } else if (op == "-") {
        return eval_minus_prefix_operator_expression(right);
    } else {
        return new object::error("unknown operator: "  + std::string(op) + right->type());
    }
}

static object::object* eval_bool_infix_expression(object::boolean* left, std::string_view op, object::boolean* right) noexcept {
    parser::trace t("eval_infix_bool_expr_method: " + left->inspect() + " " + std::string(op) + " " + right->inspect());
    if(op == "!=") {
        return (left->value() != right->value()) ? TRUE_O : FALSE_O;
    } else if(op == "==") {
        return (left->value() == right->value()) ? TRUE_O : FALSE_O;
    } else {
        return new object::error("unknown operator: " + std::string(left->type()) + " " + std::string(op) + " " + right->type());
    }
}

static object::object* eval_integer_infix_expression(object::integer* left, std::string_view op, object::integer* right) noexcept {
    parser::trace t("eval_infix_int_expr_method: " + left->inspect() + " " + std::string(op) + " " + right->inspect());
    if(op == "+") {
        return new object::integer(left->value() + right->value());
    } else if(op == "-") {
        return new object::integer(left->value() - right->value());
    } else if(op == "/") {
        return new object::integer(left->value() / right->value());
    } else if(op == "*") {
        return new object::integer(left->value() * right->value());
    } else if(op == "<") {
        return (left->value() < right->value()) ? TRUE_O : FALSE_O;
    } else if(op == ">") {
        return (left->value() > right->value()) ? TRUE_O : FALSE_O;
    } else if(op == "!=") {
        return (left->value() != right->value()) ? TRUE_O : FALSE_O;
    } else if(op == "==") {
        return (left->value() == right->value()) ? TRUE_O : FALSE_O;
    } else {
        return new object::error("unknown operator: " + std::string(left->type()) + " " + std::string(op) + " " + right->type());
    }
}

static object::object* eval_string_infix_expression(object::string* left, std::string_view op, object::string* right) noexcept {
    if(op != "+") {
        return new object::error("unknown operator: " + std::string(left->type()) + " " + std::string(op) + " " + right->type());
    }
    return new object::string(left->value() + right->value());
}

static object::object* eval_infix_expression(object::object* left, std::string_view op, object::object* right) noexcept {
    parser::trace t("eval_infix_expr_method: " + left->inspect() + " " + std::string(op) + " " + right->inspect());
    if(left->type() == object::INTEGER_OBJ && right->type() == object::INTEGER_OBJ) {
        auto* l = dynamic_cast<object::integer*>(left);
        auto* r = dynamic_cast<object::integer*>(right);
        return eval_integer_infix_expression(l, op, r);
    } else if (left->type() == object::BOOLEAN_OBJ && right->type() == object::BOOLEAN_OBJ) {
        auto* l = dynamic_cast<object::boolean*>(left);
        auto* r = dynamic_cast<object::boolean*>(right);
        return eval_bool_infix_expression(l, op, r);
    } else if (left->type() == object::STRING_OBJ && right->type() == object::STRING_OBJ) {
        auto* l = dynamic_cast<object::string*>(left);
        auto* r = dynamic_cast<object::string*>(right);
        return eval_string_infix_expression(l, op, r);
    } else if (left->type() != right->type()) {
        return new object::error("type mismatch: " + std::string(left->type()) + " " + std::string(op) + " " + right->type());
    } else {
        return new object::error("unknown operator: " + std::string(left->type()) + " " + std::string(op) + " " + right->type());
    }
}

static bool is_truthy(object::object* o) noexcept {
    object::object* obj = dynamic_cast<object::boolean*>(o);
    if (obj == NULL_O || obj == FALSE_O) {
        return false;
    } else {
        return true;
    }
}

static object::object* eval_if_expression(ast::if_expression* ie, object::scope* scope) noexcept {
    parser::trace t("eval if expr: " + ie->to_string());
    object::object* condition = eval(ie->condition(), scope);
    if (is_error(condition)) {
        return condition;
    }
    if (is_truthy(condition)) {
        return eval(ie->consequence(), scope);
    } else if (ie->alternative() != nullptr) {
        return eval(ie->alternative(), scope);
    } else {
        return NULL_O;
    }
}

static object::object* eval_block_statement(ast::block_statement* block, object::scope* scope) noexcept {
    parser::trace t("eval block statement: " + block->to_string());
    object::object* result;
    const std::vector<std::unique_ptr<ast::statement>>& stmts = block->statements();

    for(const auto& stmt : stmts){
        result = eval(stmt.get(), scope);
        if (result != nullptr) {
            object::object_t rt = result->type();
            if (rt == object::RETURN_VALUE_OBJ || rt == object::ERROR_OBJ) {
                return result;
            }
        }
    }

    return result;
}

static object::object* eval_identifier(ast::identifier* ident, object::scope* scope) {
    parser::trace t("eval_identifier: " + ident->to_string());
    if(object::object* res = scope->get(ident->value())) {
        return res;
    }
    if(object::object* builtin_fn = get_builtin(ident->value())) {
        return builtin_fn;
    }
    return new object::error("identifier not found: " + std::string(ident->value()));
}

static std::vector<object::object*> eval_expressions(const std::vector<std::unique_ptr<ast::expression>>& exps, 
        object::scope* scope) noexcept {
    parser::trace t("eval_expressions");
    std::vector<object::object*> res;

    for (const auto& exp : exps) {
        object::object* evaluated = eval(exp.get(), scope);
        if (is_error(evaluated)) {
            return std::vector<object::object*> { evaluated };
        }
        res.push_back(evaluated);
    }
    return res;
}

static object::object* unwrap_return_value(object::object* obj) noexcept {
    parser::trace t("unwrap_return_value: " + obj->inspect());
    if (auto o = dynamic_cast<object::return_value*>(obj)) {
        return o->value();
    }
    return obj;
}

static object::scope* extend_fn_scope(object::function* fn, std::vector<object::object*> args) noexcept {
    parser::trace t("extend current fn scope: " + fn->get_scope()->list_scope()); 
    object::scope* extended_scope = new object::scope(fn->get_scope());
    for (int i = 0; i < fn->parameters().size(); i++) {
        extended_scope->set(fn->parameters()[i]->value(), args[i]);
    }
    return extended_scope;
}

static object::object* apply_function(object::object* fn, std::vector<object::object*> args) noexcept {
    parser::trace t("apply function: " + fn->inspect());
    if (object::function* function = dynamic_cast<object::function*>(fn)) {
        object::scope* extended_scope = extend_fn_scope(function, args);
        object::object* evaluated = eval(function->body(), extended_scope);

        return unwrap_return_value(evaluated);
    }
    if (object::builtin* builtin_fn = dynamic_cast<object::builtin*>(fn)) {
        return builtin_fn->fn()(args);
    }

    return new object::error("not a function: " + std::string(fn->type()));
}

static object::object* eval_array_index_expression(object::object* array, object::object* index) noexcept {
    parser::trace t("eval_array_index_expr method: " + array->inspect() + " " + index->inspect());
    object::array* array_obj = dynamic_cast<object::array*>(array);
    std::int64_t idx = dynamic_cast<object::integer*>(index)->value();
    if (idx < 0 || idx > array_obj->elements().size() - 1) {
        return NULL_O;
    }
    return array_obj->elements()[idx].get();
}

static object::object* eval_index_expression(object::object* left, object::object* index) noexcept {
    parser::trace t("eval_index_expr method: " + left->inspect() + " " + index->inspect());
    if(left->type() == object::ARRAY_OBJ && index->type() == object::INTEGER_OBJ) {
        return eval_array_index_expression(left, index);
    }
    return new object::error("index operator not supported: " + std::string(left->type()) + " " + std::string(index->type()));
}

static object::object* eval(ast::node* node, object::scope* scope) {
    parser::trace t("eval");

    // Statements
    if (auto* n = dynamic_cast<ast::program*>(node)) {
        parser::trace t("eval_program");
        return eval_program(n->statements(), scope); 
    }
    if (auto* n = dynamic_cast<ast::expression_statement*>(node)) {
        parser::trace t("eval_expression_statement");
        return eval(n->expr(), scope);
    }
    
    // Expressions
    if (auto* n = dynamic_cast<ast::identifier*>(node)) {
        parser::trace t("eval_ident_expr");
        return eval_identifier(n, scope);
    }
    if (auto* n = dynamic_cast<ast::string_literal*>(node)) {
        parser::trace t("eval_ident_expr");
        return new object::string(n->value());
    }
    if (auto* n = dynamic_cast<ast::let_statement*>(node)) {
        parser::trace t("eval_let_stmt");
        object::object* val = eval(n->value(), scope);
        if (is_error(val)) {
            return val;
        }
        scope->set(n->ident().value(), val);
    }
    if (auto* n = dynamic_cast<ast::prefix_expression*>(node)) {
        parser::trace t("eval_prefix_expr");
        object::object* right = eval(n->expr(), scope);
        if (is_error(right)) {
            return right;
        }
        return eval_prefix_expression(n->op(), right);
    }
    if (auto* n = dynamic_cast<ast::infix_expression*>(node)) {
        parser::trace t("eval_infix_expr");
        object::object* left = eval(n->l_expr(), scope);
        if (is_error(left)) {
            return left;
        }
        object::object* right = eval(n->r_expr(), scope);
        if (is_error(right)) {
            return right;
        }
        return eval_infix_expression(left, n->op(), right);
    }
    if (auto* n = dynamic_cast<ast::block_statement*>(node)) {
        parser::trace t("eval_block_stmt");
        return eval_block_statement(n, scope);
    }
    if (auto* n = dynamic_cast<ast::return_statement*>(node)) {
        parser::trace t("eval_return_stmt");
        object::object* val = eval(n->return_value(), scope);
        if (is_error(val)) {
            return val;
        }
        return new object::return_value(std::unique_ptr<object::object>(val));
    }
    if (auto* n = dynamic_cast<ast::function_literal*>(node)) {
        parser::trace t("eval_fn_lit");
        return new object::function(n->parameters(), n->body(), scope);
    }
    if (auto* n = dynamic_cast<ast::call_expression*>(node)) {
        parser::trace t("eval_call_expr");
        object::object* fn = eval(n->function(), scope);
        if (is_error(fn)) {
            return fn;
        }
        std::vector<object::object*> args = eval_expressions(n->arguments(), scope);
        if (args.size() == 1 && is_error(args[0])) {
            return args[0];
        }
        return apply_function(fn, args);
    }
    if (auto* n = dynamic_cast<ast::if_expression*>(node)) {
        parser::trace t("eval_if_expr");
        return eval_if_expression(n, scope);
    }
    if (auto* n = dynamic_cast<ast::int_literal*>(node)) {
        parser::trace t("eval_int_lit: " + std::to_string(n->value()));
        return new object::integer(n->value());
    }
    if (auto* n = dynamic_cast<ast::boolean*>(node)) {
        parser::trace t("eval_boolean");
        return n->value() ? TRUE_O : FALSE_O;
    }
    if (auto* n = dynamic_cast<ast::array_literal*>(node)) {
        parser::trace t("eval_array_lit");
        std::vector<object::object*> elements = eval_expressions(n->elements(), scope);
        if(elements.size() == 1 && is_error(elements[0])) {
            return elements[0];
        }
        return new object::array(elements);
    }
    if (auto* n = dynamic_cast<ast::index_expression*>(node)) {
        parser::trace t("eval_index_expr");
        object::object* left = eval(n->left(), scope);
        if (is_error(left)){
            return left;
        }
        object::object* index = eval(n->index(), scope);
        if (is_error(index)){
            return index;
        }
        return eval_index_expression(left, index);
    }

    return nullptr;
}

} // namespace evaluator

