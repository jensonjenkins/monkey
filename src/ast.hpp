#pragma once

#include <memory>
#include <string>
#include <vector>
#include "token.hpp"

namespace ast {

struct node {
    node() noexcept = default;
    virtual ~node() noexcept = default;
    virtual const std::string to_string() const noexcept = 0; // allow std::string for debug statements
    virtual const std::string_view token_literal() const noexcept = 0;
};

struct statement : node {
    virtual const std::string_view token_literal() const noexcept = 0;
};

struct expression : node {
    virtual const std::string_view token_literal() const noexcept = 0;
};

class program : public node {
public:
    const std::string_view token_literal() const noexcept override {
        if(_statements.size() > 0) {
            return _statements[0]->token_literal();
        } else {
            return "";
        }
    }
    const std::string to_string() const noexcept override {
        std::string buf;
        for(int i=0;i<_statements.size();i++){
            ast::statement* stmt = _statements[i].get();
            buf += stmt->to_string() + '\n';
        }
        return buf;
    }
    
    const std::vector<std::unique_ptr<ast::statement>>& get_statements() const noexcept { 
        return _statements; 
    }
    void add_statement(ast::statement* stmt) noexcept { 
        _statements.push_back(std::unique_ptr<ast::statement>(stmt)); 
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

    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override { return std::string(token_literal()); }
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

    void move_value(ast::expression* expr) noexcept { _value = std::unique_ptr<expression>(expr); }

    let_statement(const let_statement& other) noexcept = delete;
    let_statement& operator=(const let_statement& other) noexcept = delete;

    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override { 
        std::string buf;
        buf = std::string(token_literal()) + " " + _ident.to_string() + " = ";
        if(_value != nullptr){
            buf += _value->to_string();
        }
        buf += ";";
        return buf; 
    }

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
    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override { 
        std::string buf;
        buf = std::string(token_literal()) + " ";
        if(_return_value != nullptr){
            buf += _return_value->to_string();
        }
        buf += ";";
        return buf; 
    }
    
protected:
    token::token                _token;
    std::unique_ptr<expression> _return_value;
};

class expression_statement : public statement {
public:    
    expression_statement(token::token token) noexcept : _token(token) {}
    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override { 
        std::string buf;
        if(_expr != nullptr){
            buf += _expr->to_string();
        }
        return buf; 
    }

    void move_expr(ast::expression* expr) noexcept { _expr = std::unique_ptr<expression>(expr); }
    expression* expr() const noexcept { return _expr.get(); } // not const, might cause trouble?

protected:
    token::token                _token; // first token of the expression
    std::unique_ptr<expression> _expr;
};

class int_literal : public expression {
public:    
    int_literal() noexcept = default;
    int_literal(token::token token, std::int64_t value) noexcept : _token(token), _value(value) {}

    int_literal(const int_literal& other) noexcept = delete;
    int_literal& operator=(const int_literal& other) noexcept = delete;
    int_literal(int_literal&& other) noexcept = default;
    int_literal& operator=(int_literal&& other) noexcept = default;

    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override { return std::to_string(_value); }


protected:
    token::token    _token; 
    std::int64_t    _value;
};

} // namespace ast

