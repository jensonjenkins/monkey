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

    /**
     * NOTE: Caller is responsible for management of ast::let_statement*
     */
    ast::program* parse_program() noexcept {
        ast::program* program = new ast::program();
 
        while(_cur_token.get_type() != token::EOFT){
            std::unique_ptr<ast::statement> stmt(parse_statement());
            if(stmt){
                program->add_statement(std::move(stmt));
            }
            next_token();
        }
        return program;
    }

    /**
     * NOTE: Caller is responsible for management of ast::statement* 
     */
    ast::statement* parse_statement() noexcept { 
        switch(_cur_token.get_type()){
        case token::LET:
            return parse_let_statement();
            break;
        default:
            return nullptr;
        }
    }

    /**
     * NOTE: Caller is responsible for management of ast::let_statement*
     */
    ast::let_statement* parse_let_statement() noexcept {
        ast::let_statement* stmt = new ast::let_statement(_cur_token);

        if(!expect_peek(token::IDENT)){
            return nullptr;
        }

        stmt->move_ident(ast::identifier(_cur_token, _cur_token.token_literal()));

        if(!expect_peek(token::ASSIGN)){
            return nullptr;
        }

        while(!cur_token_is(token::SEMICOLON)){
            next_token();
        }

        return stmt;
    }

    bool cur_token_is(token::token_t token_type) noexcept {
        return _cur_token.get_type() == token_type;
    }

    bool peek_token_is(token::token_t token_type) noexcept {
        return _peek_token.get_type() == token_type;
    }

    bool expect_peek(token::token_t token_type) noexcept {
        if(peek_token_is(token_type)){
            next_token();
            return true;
        }else{
            return false;
        }
    }

protected:
    lexer::lexer _l;
    token::token _cur_token;
    token::token _peek_token;
};

} // namespace parser

