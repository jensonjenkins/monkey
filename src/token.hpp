#pragma once

#include <cstdint>
#include <unordered_map>
#include <string_view>

namespace token { 

using token_t = std::uint16_t;

constexpr token_t ILLEGAL   = 0;
constexpr token_t EOFT      = 1;

constexpr token_t IDENT     = 2;
constexpr token_t INT       = 3;

constexpr token_t ASSIGN    = 4;
constexpr token_t PLUS      = 5;
constexpr token_t MINUS     = 6;
constexpr token_t ASTERISK  = 7;
constexpr token_t SLASH     = 8;

constexpr token_t COMMA     = 9;
constexpr token_t SEMICOLON = 10;
constexpr token_t BANG      = 11;
constexpr token_t GT        = 12;
constexpr token_t LT        = 13;
constexpr token_t EQ        = 14;
constexpr token_t NEQ       = 15;

constexpr token_t LPAREN    = 16;
constexpr token_t RPAREN    = 17;
constexpr token_t LBRACE    = 18;
constexpr token_t RBRACE    = 19;

constexpr token_t FUNCTION  = 20;
constexpr token_t LET       = 21;
constexpr token_t TRUE      = 22;
constexpr token_t FALSE     = 23;
constexpr token_t IF        = 24;
constexpr token_t ELSE      = 25;
constexpr token_t RETURN    = 26;

const std::unordered_map<std::string_view, token_t> keywords {
    {"fn", FUNCTION},
    {"let", LET},
    {"true", TRUE},
    {"false", FALSE},
    {"if", IF},
    {"else", ELSE},
    {"return", RETURN},
}; 

class token {
public: 
    token() = default;
    token(token_t t, std::string_view lit) noexcept : _type(t), _literal(lit) {};

    void set(token_t t, std::string_view lit) noexcept {
        _type = t;
        _literal = lit;
    }

    std::string_view token_literal() const noexcept { return _literal; }
    token_t get_type() const noexcept { return _type; }

private:
    token_t             _type;
    std::string_view    _literal;
};

}

// constexpr token_t ILLEGAL = "ILLEGAL";
// constexpr token_t EOFT = "EOFT";
//
// constexpr token_t IDENT = "IDENTIFIER";
// constexpr token_t INT = "INT";
//
// constexpr token_t ASSIGN = "=";
// constexpr token_t PLUS = "+";
// constexpr token_t MINUS = "-";
// constexpr token_t ASTERISK = "*";
// constexpr token_t SLASH = "/";
//
// constexpr token_t COMMA = ",";
// constexpr token_t SEMICOLON = ";";
// constexpr token_t BANG = "!";
// constexpr token_t GT = ">";
// constexpr token_t LT = "<";
// constexpr token_t EQ = "==";
// constexpr token_t NEQ = "!=";
//
// constexpr token_t LPAREN = "(";
// constexpr token_t RPAREN = ")";
// constexpr token_t LBRACE = "{";
// constexpr token_t RBRACE = "}";
//
// constexpr token_t FUNCTION = "FUNCTION";
// constexpr token_t LET = "LET";
// constexpr token_t TRUE = "TRUE";
// constexpr token_t FALSE = "FALSE";
// constexpr token_t IF = "IF";
// constexpr token_t ELSE = "ELSE";
// constexpr token_t RETURN = "RETURN";


