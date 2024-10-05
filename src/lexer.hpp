#pragma once

#include <cstdint>
#include <cstring>
#include <iostream>
#include <type_traits>
#include "token.hpp"

namespace lexer {

class lexer {
public:
    lexer() = delete;
    lexer(const char* input) : _input(input), _input_len(std::strlen(input)) {
        _peek_cursor = 0;
    };

    void read_char() {
        if (_peek_cursor >= _input_len) {
            _cur_char = 0;
        } else {
            _cur_char = _input[_peek_cursor];
        }
        _cursor = _peek_cursor;
        ++_peek_cursor;
    }
    
    token::token next_token() {
        token::token cur_token;
        read_char();

        switch (_cur_char) {
        case '=':
            cur_token.set(token::ASSIGN, &_cur_char);
            break;
        case ';':
            cur_token.set(token::SEMICOLON, &_cur_char);
            break;
        case '(':
            cur_token.set(token::LPAREN, &_cur_char);
            break;
        case ')':
            cur_token.set(token::RPAREN, &_cur_char);
            break;
        case '{':
            cur_token.set(token::LBRACE, &_cur_char);
            break;
        case '}':
            cur_token.set(token::RBRACE, &_cur_char);
            break;
        case ',':
            cur_token.set(token::COMMA, &_cur_char);
            break;
        case '+':
            cur_token.set(token::PLUS, &_cur_char);
            break;
        case 0:
            cur_token.set(token::EOFT, "");
            break;
        }

        return cur_token;
    }
    
    
private:
    const char*     _input;
    std::size_t     _input_len;
    std::uint32_t   _cursor;        // current position of the lexer
    std::uint32_t   _peek_cursor;   // current  look ahead position
    char            _cur_char;  // char pointed by _cursor 
};


} // namespace lexer

