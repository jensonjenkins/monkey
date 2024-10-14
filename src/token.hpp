#pragma once

#include <cstdint>
#include <unordered_map>
#include <array>
#include <string_view>

namespace token { 

using token_t = std::uint32_t;

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

const std::array<std::string_view, 27> inv_map {
    "ILLEGAL", "EOFT", 
    "IDENT", "INT", 
    "ASSIGN", "PLUS", "MINUS", "ASTERISK", "SLASH", 
    "COMMA", "SEMICOLON", "BANG", "GT", "LT", "EQ", "NEQ", 
    "LPAREN", "RPAREN", "LBRACE", "RBRACE",
    "FUNCTION", "LET", "TRUE", "FALSE", "IF", "ELSE", "RETURN"
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


