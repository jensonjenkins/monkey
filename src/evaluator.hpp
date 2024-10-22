#pragma once

#include "trace.hpp"
#include "object.hpp"
#include "ast.hpp"

namespace evaluator {

inline object::null* NULL_O = new object::null;
inline object::boolean* TRUE_O = new object::boolean(true);
inline object::boolean* FALSE_O = new object::boolean(false);

static object::object* eval(ast::node* node); // forward declaration to avoid compiler complaints

static object::object* eval_statements (const std::vector<std::unique_ptr<ast::statement>>& stmts) {
    object::object* result;
    
    for(const auto& stmt : stmts){
        result = eval(stmt.get());
    }

    return result;
}

static object::object* eval_bang_operator_expression(object::object* right) noexcept {
    parser::trace t("eval_bang_operator_expr: " + right->inspect());
    if(right == TRUE_O) {
        return FALSE_O;
    } else if(right == FALSE_O){
        return TRUE_O;
    } else if(right == NULL_O){
        return TRUE_O;
    } else {
        return FALSE_O;
    }
}

static object::object* eval_minus_prefix_operator_expression(object::object* right) noexcept {
    if(right->type() != object::INTEGER_OBJ) {
        return NULL_O;
    }
    object::integer* r = dynamic_cast<object::integer*>(right);
    return new object::integer(-(r->value()));
}

static object::object* eval_prefix_expression(std::string_view op, object::object* right) {
    parser::trace t("eval_prefix_expression_method: " + std::string(op) + " " + right->inspect());
    if(op == "!") {
        return eval_bang_operator_expression(right);
    } else if(op == "-") {
        return eval_minus_prefix_operator_expression(right);
    } else {
        return NULL_O;
    }
}

static object::object* eval_bool_infix_expression(object::boolean* left, std::string_view op, object::boolean* right) noexcept {
    parser::trace t("eval_infix_bool_expr_method: " + left->inspect() + " " + std::string(op) + " " + right->inspect());
    if(op == "!=") {
        return (left->value() != right->value()) ? TRUE_O : FALSE_O;
    } else if(op == "==") {
        return (left->value() == right->value()) ? TRUE_O : FALSE_O;
    } else {
        return NULL_O;
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
        return new object::boolean(left->value() < right->value());
    } else if(op == ">") {
        return new object::boolean(left->value() > right->value());
    } else if(op == "!=") {
        return new object::boolean(left->value() != right->value());
    } else if(op == "==") {
        return new object::boolean(left->value() == right->value());
    } else {
        return NULL_O;
    }
}

static object::object* eval_infix_expression(object::object* left, std::string_view op, object::object* right) {
    parser::trace t("eval_infix_expr_method: " + left->inspect() + " " + std::string(op) + " " + right->inspect());
    if(left->type() == object::INTEGER_OBJ && right->type() == object::INTEGER_OBJ) {
        auto* l = dynamic_cast<object::integer*>(left);
        auto* r = dynamic_cast<object::integer*>(right);
        return eval_integer_infix_expression(l, op, r);
    } else if (left->type() == object::BOOLEAN_OBJ && right->type() == object::BOOLEAN_OBJ) {
        auto* l = dynamic_cast<object::boolean*>(left);
        auto* r = dynamic_cast<object::boolean*>(right);
        return eval_bool_infix_expression(l, op, r);
    } else {
        return NULL_O;
    }
}

static object::object* eval(ast::node* node) {
    parser::trace t("eval");

    // Statements
    if (auto* n = dynamic_cast<ast::program*>(node)) {
        parser::trace t("eval_program");
        return eval_statements(n->statements()); 
    }
    if (auto* n = dynamic_cast<ast::expression_statement*>(node)) {
        parser::trace t("eval_expression_statement");
        return eval(n->expr());
    }
    
    // Expressions
    if (auto* n = dynamic_cast<ast::prefix_expression*>(node)) {
        parser::trace t("eval_prefix_expression");
        object::object* right = eval(n->expr());
        return eval_prefix_expression(n->op(), right);
    }
    if (auto* n = dynamic_cast<ast::infix_expression*>(node)) {
        parser::trace t("eval_infix_expression");
        object::object* left = eval(n->l_expr());
        object::object* right = eval(n->r_expr());
        return eval_infix_expression(left, n->op(), right);
    }
    if (auto* n = dynamic_cast<ast::int_literal*>(node)) {
        parser::trace t("eval_int_literal: " + std::to_string(n->value()));
        return new object::integer(n->value());
    }
    if (auto* n = dynamic_cast<ast::boolean*>(node)) {
        parser::trace t("eval_boolean");
        return n->value() ? TRUE_O : FALSE_O;
    }

    return nullptr;
}

} // namespace evaluator

