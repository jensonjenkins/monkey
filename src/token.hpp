#pragma once

#include <unordered_map>
#include <string_view>

namespace token { 

using token_t = std::string_view;

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

const std::unordered_map<std::string_view, token_t> keywords {
    {"fn", FUNCTION},
    {"let", LET}
}; 

class token {
public: 
    token() = default;
    token(token_t t, std::string_view lit) noexcept : _type(t), _literal(lit) {};

    void set(token_t t, std::string_view lit) noexcept {
        _type = t;
        _literal = lit;
    }

    token_t             _type;
    std::string_view    _literal;
};

}
