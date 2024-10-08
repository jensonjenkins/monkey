#pragma once

#include <memory>
#include <vector>
#include "token.hpp"

namespace ast {

struct node {
public:
    virtual std::string_view token_literal() = 0;
};

class statement : node {
public:
    virtual void statement_node() = 0; 
    virtual std::string_view token_literal() = 0;
};

class expression : node {
public:
    virtual void expression_node() = 0; 
    virtual std::string_view token_literal() = 0;
};

class program : node {
public:
    program() noexcept = default;
    std::string_view token_literal() override {
        if(_statements.size() > 0) {
            return _statements[0]->token_literal();
        } else {
            return "";
        }
    };

    std::vector<std::unique_ptr<ast::statement>> get_statements() const noexcept { return _statements; }
    void set_statements(std::vector<std::unique_ptr<ast::statement>>&& stmts) noexcept {
        _statements = std::move(stmts);
    }
    void add_statement(std::unique_ptr<ast::statement>&& stmt) noexcept {
        _statements.push_back(std::move(stmt));
    }

protected: 
    std::vector<std::unique_ptr<ast::statement>> _statements;
};

class identifier : expression {
public:
    void expression_node() override;

    std::string_view token_literal() noexcept override { return _token.token_literal(); }
    const char* get_value() const noexcept { return _value; }

protected:
    token::token    _token;
    const char*     _value;
};

class let_statement : statement {
public:
    void statement_node() override;
    std::string_view token_literal() override {
        return _token.token_literal();
    };

    ast::identifier get_ident() const noexcept { return _ident; }
    token::token get_token() const noexcept { return _token; }

protected:
    token::token                _token;
    ast::identifier             _ident;
    std::unique_ptr<expression> _value;
};

} // namespace ast

