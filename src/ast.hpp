#pragma once

#include <memory>
#include <vector>
#include "token.hpp"

namespace ast {

struct node {
    node() noexcept = default;
    virtual ~node() noexcept = default;
    virtual std::string_view token_literal() const noexcept = 0;
};

struct statement : node {
    virtual std::string_view token_literal() const noexcept = 0;
};

struct expression : node {
    virtual std::string_view token_literal() const noexcept = 0;
};

class program : public node {
public:
    std::string_view token_literal() const noexcept override {
        if(_statements.size() > 0) {
            return _statements[0]->token_literal();
        } else {
            return "";
        }
    }

    const std::vector<std::unique_ptr<ast::statement>>& get_statements() const noexcept { 
        return _statements; 
    }
    void add_statement(std::unique_ptr<ast::statement>&& stmt) noexcept { 
        _statements.push_back(std::move(stmt)); 
    }

protected: 
    /**
     * Rationale is that the program owns all its statements
     */
    std::vector<std::unique_ptr<ast::statement>> _statements;
};

class identifier : public expression {
public:
    identifier() noexcept = default;
    identifier(token::token token, std::string_view value) noexcept : _token(token), _value(value) {}
    
    identifier(const identifier& other) noexcept = delete;
    identifier& operator=(const identifier& other) noexcept = delete;
    identifier(identifier&& other) noexcept = default;
    identifier& operator=(identifier&& other) noexcept = default;

    std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    std::string_view get_value() const noexcept { return _value; }

protected:
    token::token         _token;
    std::string_view     _value;
};

class let_statement : public statement {
public:
    let_statement(token::token token) noexcept : _token(token) {}

    void move_ident(ast::identifier&& ident) { 
        _ident = std::move(ident);
    }

    let_statement(const let_statement& other) noexcept = delete;
    let_statement& operator=(const let_statement& other) noexcept = delete;

    std::string_view token_literal() const noexcept override { return _token.token_literal(); }

    const ast::identifier& get_ident() const noexcept { return _ident; }
    const token::token& get_token() const noexcept { return _token; }

protected:
    token::token                _token;
    ast::identifier             _ident;
    std::unique_ptr<expression> _value;
};

class return_statement : public statement { 
public:
    return_statement(token::token token) noexcept : _token(token) {}
    std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    
protected:
    token::token                _token;
    std::unique_ptr<expression> _value;
};

} // namespace ast

