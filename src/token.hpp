#pragma once

namespace token { 

using token_t = const char*;

class token {
public: 
    token() = default;
    token(token_t type, char* literal) noexcept : _type(type), _literal(literal) {};

    void set(token_t type, const char* literal) noexcept {
        _type = type;
        _literal = literal;
    }

    token_t     _type;
    const char* _literal;
};

constexpr token_t ILLEGAL = "ILLEGAL";
constexpr token_t EOFT = "EOFT";

constexpr token_t IDENT = "IDENTIFIER";
constexpr token_t INT = "INT";

constexpr token_t ASSIGN = "=";
constexpr token_t PLUS = "+";

constexpr token_t COMMA = ",";
constexpr token_t SEMICOLON = ";";

constexpr token_t LPAREN = "(";
constexpr token_t RPAREN = ")";
constexpr token_t LBRACE = "{";
constexpr token_t RBRACE = "}";

constexpr token_t FUNCTION = "FUNCTION";
constexpr token_t LET = "LET";

}
