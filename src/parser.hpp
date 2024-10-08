#pragma once

#include "token.hpp"
#include "ast.hpp"
#include "lexer.hpp"


namespace parser {

class parser {
public:
    void next_token() noexcept {
        _cur_token = _peek_token;
        _peek_token = _l.next_token();
    }

    parser(lexer::lexer l) noexcept : _l(l) {
        next_token();
        next_token(); // initialize both _cur and _peek tokens
    }

    ast::program* parse_program() noexcept {
        ast::program program;
        program.set_statements(std::vector<std::unique_ptr<ast::statement>>());
        
        while(_cur_token.get_type() != token::EOFT){
            ast::statement* stmt = parse_statement();
            if(stmt != nullptr){
                program.add_statement(std::unique_ptr<ast::statement>(stmt));
            }
            next_token();
        }
        return nullptr;
    }

    ast::statement* parse_statement() noexcept { 
        return nullptr; 
    }

protected:
    lexer::lexer _l;
    token::token _cur_token;
    token::token _peek_token;
};

} // namespace parser

