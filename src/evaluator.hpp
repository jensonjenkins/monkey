#pragma once

#include "object.hpp"
#include "ast.hpp"

namespace evaluator {

inline object::null* NULL_O= new object::null;
inline object::boolean* TRUE_O = new object::boolean(true);
inline object::boolean* FALSE_O= new object::boolean(false);

static object::object* eval(ast::node* node); // forward declaration to avoid compiler complaints

static object::object* eval_statements (const std::vector<std::unique_ptr<ast::statement>>& stmts) {
    object::object* result;
    
    for(const auto& stmt : stmts){
        result = eval(stmt.get());
    }

    return result;
}

static object::object* eval_bang_operator_expression(object::object* right) noexcept {
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

static object::object* eval_prefix_expression(std::string_view op, object::object* right) {
    if(op == "!") {
        return eval_bang_operator_expression(right);
    } else {
        return NULL_O;
    }
}


static object::object* eval(ast::node* node) {

    // Statements
    if (auto* n = dynamic_cast<ast::program*>(node)) {
        return eval_statements(n->statements()); 
    }
    if (auto* n = dynamic_cast<ast::expression_statement*>(node)) {
        return eval(n->expr());
    }
    
    // Expressions
    if (auto* n = dynamic_cast<ast::prefix_expression*>(node)) {
        object::object* right = eval(n->expr());
        return eval_prefix_expression(n->op(), right);
    }
    if (auto* n = dynamic_cast<ast::int_literal*>(node)) {
        return new object::integer(n->value());
    }
    if (auto* n = dynamic_cast<ast::boolean*>(node)) {
        return n->value() ? TRUE_O : FALSE_O;
    }

    return nullptr;
}

} // namespace evaluator

