#pragma once

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
    std::string_view token_literal() override {
        if(statements.size() > 0) {
            return statements[0].token_literal();
        } else {
            return "";
        }
    };

protected:
    std::vector<statement> statements;
};

class identifier : expression {
public:
    void expression_node() override;
    std::string_view token_literal() override {
        return token._literal;
    };

protected:
    token::token token;
    const char* value;
};

class let_statement : statement {
public:
    void statement_node() override;
    std::string_view token_literal() override {
        return token._literal;
    };

protected:
    token::token token;
    identifier name;
    std::unique_ptr<expression> value;
};


} // namespace ast

