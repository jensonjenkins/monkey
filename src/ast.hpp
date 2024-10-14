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
    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override { 
        std::string buf;
        if(_expr != nullptr){
            buf += _expr->to_string();
        }
        return buf; 
    }
protected:
    token::token                _token; // first token of the expression
    std::unique_ptr<expression> _expr;
};

} // namespace ast

