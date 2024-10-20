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
    
    const std::vector<std::unique_ptr<ast::statement>>& statements() const noexcept { 
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
    identifier(token::token token, std::string_view value) noexcept : _token(token), _value(std::string(value)) {}
    
    identifier(const identifier& other) noexcept = delete;
    identifier& operator=(const identifier& other) noexcept = delete;
    identifier(identifier&& other) noexcept = default;
    identifier& operator=(identifier&& other) noexcept = default;

    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override { return std::string(token_literal()); }
    std::string_view value() const noexcept { return _value; }

protected:
    token::token    _token;
    std::string     _value;
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

    const ast::identifier& ident() const noexcept { return _ident; }
    const token::token& token() const noexcept { return _token; }

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
    std::int64_t value() const noexcept { return _value; }

protected:
    token::token    _token; 
    std::int64_t    _value;
};

class prefix_expression : public expression {
public:
    prefix_expression() noexcept = default;
    prefix_expression(token::token token, std::string_view op) noexcept : _token(token), _op(std::string(op)) {}
    
    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override {
        std::string buf;
        buf += "(";
        buf += std::string(_op);
        buf += _expr->to_string();
        buf += ")";
        return buf;
    }

    std::string_view op() const noexcept { return _op; }
    expression* expr() const noexcept { return _expr.get(); }
    void set_expr(ast::expression* expr) noexcept { _expr = std::unique_ptr<ast::expression>(expr); }
    
protected:
    token::token                        _token;
    std::string                         _op;
    std::unique_ptr<ast::expression>    _expr; 
};

class infix_expression : public expression {
public: 
    infix_expression() noexcept = default;
    infix_expression(token::token token, std::string_view op, ast::expression* l_expr) noexcept
        : _token(token), _op(std::string(op)), _l_expr(l_expr) {}

    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override {
        std::string buf;
        buf += "(";
        buf += _l_expr->to_string();
        buf += " " + std::string(_op) + " ";
        buf += _r_expr->to_string();
        buf += ")";
        return buf;
    }

    std::string_view op() const noexcept { return _op; }
    expression* l_expr() const noexcept { return _l_expr.get(); }
    expression* r_expr() const noexcept { return _r_expr.get(); }
    void left_expr(ast::expression* l_expr) { _l_expr = std::unique_ptr<ast::expression>(l_expr); }
    void right_expr(ast::expression* r_expr) { _r_expr = std::unique_ptr<ast::expression>(r_expr); }

protected:
    token::token                        _token; // the operator token (e.g. +, -, etc.)
    std::string                         _op;
    std::unique_ptr<ast::expression>    _l_expr;
    std::unique_ptr<ast::expression>    _r_expr;
};

class boolean : public expression {
public:
    boolean() noexcept = default;
    boolean(token::token token, bool value) noexcept : _token(token), _value(value) {};
    
    const bool value() const noexcept { return _value; }

    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override { return std::string(_token.token_literal()); }
    
protected:
    token::token    _token; 
    bool            _value;
};

class block_statement : public statement {
public:
    const std::vector<std::unique_ptr<ast::statement>>& statements() const noexcept { return _statements; }

    void add_statement(ast::statement* stmt) noexcept { 
        _statements.push_back(std::unique_ptr<ast::statement>(stmt)); 
    }

    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override {
        std::string buf;
        for(int i=0;i<_statements.size();i++){
            ast::statement* stmt = _statements[i].get();
            buf += stmt->to_string() + '\n';
        }
        return buf;
    }

protected:
    token::token                                    _token; // the '{' token
    std::vector<std::unique_ptr<ast::statement>>    _statements;
};

class if_expression : public expression {
public:
    if_expression() noexcept = default;
    if_expression(token::token token) noexcept : _token(token) {}
    
    ast::expression* condition() const noexcept { return _condition.get(); }
    ast::block_statement* consequence() const noexcept { return _consequence.get(); }
    ast::block_statement* alternative() const noexcept { return _alternative.get(); }

    void set_condition(ast::expression* expr) noexcept { _condition = std::unique_ptr<ast::expression>(expr); }
    void set_consequence(ast::block_statement* consequence) noexcept { 
        _consequence = std::unique_ptr<ast::block_statement>(consequence);
    }
    void set_alternative(ast::block_statement* alternative) noexcept {
        _alternative = std::unique_ptr<ast::block_statement>(alternative);
    }

    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override {
        std::string buf;
        buf += "if";
        buf += _condition->to_string();
        buf += " ";
        buf += _consequence->to_string();
        if(_alternative != nullptr) {
            buf += "else ";
            buf += _alternative->to_string();
        }
        return buf;
    }

protected:
    token::token                            _token; // the 'if' token
    std::unique_ptr<ast::expression>        _condition;
    std::unique_ptr<ast::block_statement>   _consequence;
    std::unique_ptr<ast::block_statement>   _alternative;
};

class function_literal : public expression {
public:
    function_literal() noexcept = default;
    function_literal(token::token token) noexcept : _token(token) {};
    
    const std::vector<std::unique_ptr<ast::identifier>>& parameters() const noexcept { return _parameters; }
    const ast::block_statement* body() const noexcept { return _body.get(); }

    void set_parameters(std::vector<ast::identifier*> stmt) noexcept {
        for(ast::identifier* s : stmt) {
            _parameters.push_back(std::unique_ptr<ast::identifier>(s));
        }
    }
    void set_body(ast::block_statement* stmt) noexcept { _body = std::unique_ptr<ast::block_statement>(stmt); }

    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override {
        std::string buf;
        buf += _token.token_literal();
        buf += "(";
        for(int i=0;i<_parameters.size();i++){
            ast::identifier* ident = _parameters[i].get();
            buf += ident->to_string() + ',';
        }
        buf += ")";
        buf += _body->to_string();
        return buf;
    }

protected:
    token::token                                    _token;
    std::unique_ptr<ast::block_statement>           _body;
    std::vector<std::unique_ptr<ast::identifier>>   _parameters;
};

class call_expression : public expression {
public:
    call_expression() noexcept = default;
    call_expression(token::token token, ast::expression* function) noexcept : _token(token) { set_function(function); }

    const std::vector<std::unique_ptr<ast::expression>>& arguments() const noexcept { return _arguments; }
    ast::expression* function() const noexcept { return _function.get(); }

    void set_arguments(std::vector<ast::expression*> stmt) noexcept {
        for(ast::expression* s : stmt) {
            _arguments.push_back(std::unique_ptr<ast::expression>(s));
        }
    }
    void set_function(ast::expression* stmt) noexcept { _function = std::unique_ptr<ast::expression>(stmt); }
    
    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override {
        std::string buf;
        buf += _function->token_literal();
        buf += "(";
        for(int i=0;i<_arguments.size();i++){
            ast::expression* expr= _arguments[i].get();
            buf += expr->to_string();
            if(i != _arguments.size() - 1) {  buf += ", "; }
        }
        buf += ")";
        return buf;
    }
    
protected:
    token::token                                    _token; // the '(' token
    std::unique_ptr<ast::expression>                _function; // ident or fn literal
    std::vector<std::unique_ptr<ast::expression>>   _arguments;
};

} // namespace ast

