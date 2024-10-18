#pragma once

#include <cctype>
#include <cstdint>
#include <cstring>
#include <string_view>
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
        read_char();
        if(std::isspace(_cur_char)) {
            skip_whitespace();
        }

        token::token cur_token;
        std::string_view cur_char(&_cur_char, 1);

        switch (_cur_char) {
        case '=':
            if(peek_char() == '='){
                read_char();
                cur_token.set(token::EQ, "==");
            } else {
                cur_token.set(token::ASSIGN, cur_char);
            }
            break;
        case '+':
            cur_token.set(token::PLUS, cur_char);
            break;
        case '-':
            cur_token.set(token::MINUS, cur_char);
            break;
        case '*':
            cur_token.set(token::ASTERISK, cur_char);
            break;
        case '/':
            cur_token.set(token::SLASH, cur_char);
            break;
        case ',':
            cur_token.set(token::COMMA, cur_char);
            break;
        case ';':
            cur_token.set(token::SEMICOLON, cur_char);
            break;
        case '!':
            if(peek_char() == '='){
                read_char();
                cur_token.set(token::NEQ, "!=");
            } else {
                cur_token.set(token::BANG, cur_char);
            }
            break;
        case '>':
            cur_token.set(token::GT, cur_char);
            break;
        case '<':
            cur_token.set(token::LT, cur_char);
            break;
        case '(':
            cur_token.set(token::LPAREN, cur_char);
            break;
        case ')':
            cur_token.set(token::RPAREN, cur_char);
            break;
        case '{':
            cur_token.set(token::LBRACE, cur_char);
            break;
        case '}':
            cur_token.set(token::RBRACE, cur_char);
            break;
        case 0:
            cur_token.set(token::EOFT, "");
            break;
        default: 
            if(is_letter(_cur_char)) {
                std::string_view ident = read_identifier();
                cur_token.set(lookup_ident(ident), ident);
            }else if(std::isdigit(_cur_char)) {
                std::string_view digits = read_digits();
                cur_token.set(token::INT, digits);
            }else{
                cur_token.set(token::ILLEGAL, cur_char);
            }
        }
        
        return cur_token;
    }

    char peek_char() const noexcept {
        if(_peek_cursor >= _input_len) {   
            return 0;
        } else {
            return _input[_peek_cursor];
        }
    }
    
    bool is_letter(char ch) const noexcept {
        return 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_';
    }

    void skip_whitespace() noexcept {
        while(std::isspace(_cur_char)) { 
            read_char(); 
        }
    }

    std::string_view read_identifier() noexcept {
        std::uint32_t start = _cursor;
        while(is_letter(peek_char())) { 
            read_char(); 
        }
        std::string_view identifier(_input + start, _cursor - start + 1);

        return identifier;
    }

    std::string_view read_digits() noexcept {
        std::uint32_t start = _cursor;
        while(std::isdigit(peek_char())) { 
            read_char(); 
        }
        std::string_view digits(_input + start, _cursor - start + 1);

        return digits;
    }

    token::token_t lookup_ident(std::string_view ident) noexcept {
        auto it = token::keywords.find(ident);
        if(it != token::keywords.end()){
            return it->second;
        }
        return token::IDENT;
    }
    
private:
    const char*     _input;
    std::size_t     _input_len;
    std::uint32_t   _cursor;        // current position of the lexer
    std::uint32_t   _peek_cursor;   // current  look ahead position
    char            _cur_char;      // char pointed by _cursor 
};


} // namespace lexer

