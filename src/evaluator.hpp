#pragma once

#include "object.hpp"
#include "ast.hpp"

namespace evaluator {

static object::object* eval(ast::node* node); // forward declaration to avoid compiler complaints

static object::object* eval_statements (const std::vector<std::unique_ptr<ast::statement>>& stmts) {
    object::object* result;
    
    for(const auto& stmt : stmts){
        result = eval(stmt.get());
    }

    return result;
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
    if (auto* n = dynamic_cast<ast::int_literal*>(node)) {
        return new object::integer(n->value());
    }

    return nullptr;
}

} // namespace evaluator

