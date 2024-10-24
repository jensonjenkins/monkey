#pragma once

#include "trace.hpp"
#include "object.hpp"
#include "scope.hpp"
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
    object::object* res = scope->get(ident->value());
    if(res == nullptr) {
        return new object::error("identifier not found: " + std::string(ident->value()));
    }
    return res;
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
        return eval_identifier(n, scope);
    }
    if (auto* n = dynamic_cast<ast::let_statement*>(node)) {
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
    if (auto* n = dynamic_cast<ast::if_expression*>(node)) {
        parser::trace t("eval_if_expr");
        return eval_if_expression(n, scope);
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

