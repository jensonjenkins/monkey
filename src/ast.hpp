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
            statement* stmt = _statements[i].get();
            buf += stmt->to_string();
        }
        return buf;
    }
    
    const std::vector<std::shared_ptr<statement>>& statements() const noexcept { return _statements; }

    void add_statement(statement* stmt) noexcept { 
        _statements.push_back(std::shared_ptr<statement>(stmt)); 
    }
    void add_statement(std::shared_ptr<statement>&& stmt) noexcept { _statements.push_back(std::move(stmt)); }

protected: 
    /**
     * Rationale is that the program owns all its statements
     */
    std::vector<std::shared_ptr<statement>> _statements;
};

class identifier : public expression {
public:
    identifier() noexcept = default;
    identifier(token::token token, std::string_view value) noexcept : _token(token), _value(std::string(value)) {}
    ~identifier() noexcept = default;
    
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
    let_statement(const let_statement& other) noexcept = delete;
    let_statement& operator=(const let_statement& other) noexcept = delete;
    ~let_statement() noexcept = default;

    void move_ident(identifier&& ident) { _ident = std::move(ident); }
    void set_value(expression* expr) noexcept { _value = std::shared_ptr<expression>(expr); }

    std::shared_ptr<const expression> value() const noexcept { return _value; }

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

    const identifier& ident() const noexcept { return _ident; }
    const token::token& token() const noexcept { return _token; }

protected:
    token::token                _token;
    identifier                  _ident;
    std::shared_ptr<expression> _value;
};

class return_statement : public statement { 
public:
    return_statement(token::token token) noexcept : _token(token) {}

    std::shared_ptr<const expression> return_value() const noexcept { return _return_value; }
    void set_return_value(expression* rv) noexcept { _return_value = std::shared_ptr<expression>(rv); }

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
    std::shared_ptr<expression> _return_value;
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

    void move_expr(expression* expr) noexcept { _expr = std::shared_ptr<expression>(expr); }
    std::shared_ptr<const expression> expr() const noexcept { return _expr; } 

protected:
    token::token                _token; // first token of the expression
    std::shared_ptr<expression> _expr;
};

class int_literal : public expression {
public:    
    int_literal() noexcept = default;
    int_literal(token::token token, std::int64_t value) noexcept : _token(token), _value(value) {}
    ~int_literal() noexcept = default;

    int_literal(const int_literal& other) noexcept = delete;
    int_literal& operator=(const int_literal& other) noexcept = delete;

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
    std::shared_ptr<const expression> expr() const noexcept { return _expr; }
    void set_expr(expression* expr) noexcept { _expr = std::shared_ptr<expression>(expr); }
    
protected:
    token::token                    _token;
    std::string                     _op;
    std::shared_ptr<expression>     _expr; 
};

class infix_expression : public expression {
public: 
    infix_expression() noexcept = default;
    infix_expression(token::token token, std::string_view op, expression* l_expr) noexcept
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
    std::shared_ptr<const expression> l_expr() const noexcept { return _l_expr; }
    std::shared_ptr<const expression> r_expr() const noexcept { return _r_expr; }
    void left_expr(expression* l_expr) { _l_expr = std::shared_ptr<expression>(l_expr); }
    void right_expr(expression* r_expr) { _r_expr = std::shared_ptr<expression>(r_expr); }

protected:
    token::token                    _token; // the operator token (e.g. +, -, etc.)
    std::string                     _op;
    std::shared_ptr<expression>     _l_expr;
    std::shared_ptr<expression>     _r_expr;
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
    const std::vector<std::shared_ptr<statement>>& statements() const noexcept { return _statements; }

    void add_statement(statement* stmt) noexcept { _statements.push_back(std::shared_ptr<statement>(stmt)); }

    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override {
        std::string buf;
        for(int i=0;i<_statements.size();i++){
            buf += _statements[i]->to_string();
        }
        return buf;
    }

protected:
    token::token                                    _token; // the '{' token
    std::vector<std::shared_ptr<statement>>    _statements;
};

class if_expression : public expression {
public:
    if_expression() noexcept = default;
    if_expression(token::token token) noexcept : _token(token) {}
    
    std::shared_ptr<const expression> condition() const noexcept { return _condition; }
    std::shared_ptr<const block_statement> consequence() const noexcept { return _consequence; }
    std::shared_ptr<const block_statement> alternative() const noexcept { return _alternative; }

    void set_condition(expression* expr) noexcept { _condition = std::shared_ptr<expression>(expr); }
    void set_consequence(block_statement* consequence) noexcept { 
        _consequence = std::shared_ptr<block_statement>(consequence);
    }
    void set_alternative(block_statement* alternative) noexcept {
        _alternative = std::shared_ptr<block_statement>(alternative);
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
    std::shared_ptr<expression>        _condition;
    std::shared_ptr<block_statement>   _consequence;
    std::shared_ptr<block_statement>   _alternative;
};

class function_literal : public expression {
public:
    function_literal() noexcept = default;
    function_literal(token::token token) noexcept : _token(token) {};
    
    const std::vector<std::shared_ptr<identifier>>& parameters() const noexcept { return _parameters; }
    std::vector<std::shared_ptr<identifier>> move_parameters() const noexcept { return std::move(_parameters); }
    std::shared_ptr<const block_statement> body() const noexcept { return _body; }

    void set_parameters(std::vector<identifier*> stmt) noexcept {
        _parameters.clear();
        for(identifier* s : stmt) {
            _parameters.push_back(std::shared_ptr<identifier>(s));
        }
    }
    void set_body(block_statement* stmt) noexcept { _body = std::shared_ptr<block_statement>(stmt); }

    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override {
        std::string buf;
        buf += _token.token_literal();
        buf += "(";
        for(int i=0;i<_parameters.size();i++){
            buf += _parameters[i]->to_string() + ',';
        }
        buf += ")";
        buf += _body->to_string();
        return buf;
    }

protected:
    token::token                                        _token;
    std::shared_ptr<block_statement>                    _body;
    mutable std::vector<std::shared_ptr<identifier>>    _parameters; // marked mutable to be moved to object::function
                                                                     // so that when the ast::fn_literal object is
                                                                     // deallocated, the identifiers don't get deallocated
                                                                     // with it.
};

class call_expression : public expression {
public:
    call_expression() noexcept = default;
    call_expression(token::token token, expression* function) noexcept : _token(token) { set_function(function); }

    const std::vector<std::shared_ptr<expression>>& arguments() const noexcept { return _arguments; }

    std::shared_ptr<const expression> function() const noexcept { return _function; }

    void set_arguments(std::vector<expression*> stmt) noexcept {
        _arguments.clear();
        for(expression* s : stmt) {
            _arguments.push_back(std::shared_ptr<expression>(s));
        }
    }
    void set_function(expression* stmt) noexcept { _function = std::shared_ptr<expression>(stmt); }
    
    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override {
        std::string buf;
        buf += _function->token_literal();
        buf += "(";
        for(int i=0;i<_arguments.size();i++){
            buf += _arguments[i]->to_string();
            if(i != _arguments.size() - 1) { buf += ", "; }
        }
        buf += ")";
        return buf;
    }
    
protected:
    token::token                                _token; // the '(' token
    std::shared_ptr<expression>                 _function; // ident or fn literal
    std::vector<std::shared_ptr<expression>>    _arguments;
};

class string_literal : public expression {
public:
    string_literal() noexcept = default;
    string_literal(token::token token, std::string_view value) noexcept : _token(token), _value(value) {}

    const std::string& value() const noexcept { return _value; }

    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override { return _value; }

protected:
    token::token    _token;
    std::string     _value;
};

class array_literal : public expression {
public:
    array_literal(token::token token) noexcept : _token(token) {}

    const std::vector<std::shared_ptr<expression>>& elements() const noexcept { return _elements; }

    void set_elements(std::vector<expression*> expr_list) noexcept {
        _elements.clear();
        for(expression* expr : expr_list) {
            _elements.push_back(std::shared_ptr<expression>(expr));
        }
    }

    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override {
        std::string buf;
        buf += "[";
        for(int i=0;i<_elements.size();i++){
            buf += _elements[i]->to_string();
            if(i != _elements.size() - 1) { buf += ", "; }
        }
        buf += "]";
        return buf;
    }

protected:
    token::token                                _token; // the '[ token
    std::vector<std::shared_ptr<expression>>    _elements;

}; 

class index_expression : public expression {
public:
    index_expression(token::token token, expression* left) noexcept : _token(token) { set_left(left); }

    std::shared_ptr<const expression> left() const noexcept { return _left; }
    std::shared_ptr<const expression> index() const noexcept { return _index; }

    void set_left(expression* left) noexcept { _left = std::shared_ptr<expression>(left); }
    void set_index(expression* index) noexcept { _index = std::shared_ptr<expression>(index); }

    const std::string_view token_literal() const noexcept override { return _token.token_literal(); }
    const std::string to_string() const noexcept override {
        std::string buf;
        buf += "(";
        buf += _left->to_string();
        buf += "[";
        buf += _index->to_string();
        buf += "])";
        return buf;
    }

protected: 
    token::token                        _token; // the '[ token
    std::shared_ptr<expression>         _left;
    std::shared_ptr<expression>         _index;
};

} // namespace ast



