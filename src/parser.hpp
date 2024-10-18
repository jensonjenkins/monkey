#pragma once

#include "token.hpp"
#include "ast.hpp"
#include "lexer.hpp"
#include <cstdint>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace parser {

using precedence = uint8_t;

constexpr precedence LOWEST         = 0; // placeholder only
constexpr precedence EQUALS         = 1; // ==
constexpr precedence LESSGREATER    = 2; // < or >
constexpr precedence SUM            = 3; // + 
constexpr precedence PRODUCT        = 4; // *
constexpr precedence PREFIX         = 5; // -var or !var
constexpr precedence CALL           = 6; // my_fn(var)

const std::unordered_map<token::token_t, precedence> precedences {
    {token::EQ,         EQUALS},
    {token::NEQ,        EQUALS},
    {token::LT,         LESSGREATER},
    {token::GT,         LESSGREATER},
    {token::PLUS,       SUM},
    {token::MINUS,      SUM},
    {token::SLASH,      PRODUCT},
    {token::ASTERISK,   PRODUCT},
};

class parser {
public:
    using prefix_parse_fn_t = std::function<ast::expression*()>;
    using infix_parse_fn_t = std::function<ast::expression*(ast::expression*)>;

    void next_token() noexcept {
        _cur_token = _peek_token;
        token::token _cur_token_assertion = _cur_token; // only for the assertion statement below
        _peek_token = _l.next_token();

        assert(_cur_token == _cur_token_assertion && "cur_token changed when next_token() is called");
    }

    parser(lexer::lexer l) noexcept : _l(l) {
        next_token();
        next_token(); // initialize both _cur and _peek tokens

        register_prefix_fn(token::IDENT,    [this]() -> ast::expression* { return this->parse_identifier(); });
        register_prefix_fn(token::INT,      [this]() -> ast::expression* { return this->parse_int_literal(); });
        register_prefix_fn(token::BANG,     [this]() -> ast::expression* { return this->parse_prefix_expr(); });
        register_prefix_fn(token::MINUS,    [this]() -> ast::expression* { return this->parse_prefix_expr(); });

        register_infix_fn(token::PLUS,      [this](ast::expression* a) -> ast::expression* { return this->parse_infix_expr(a); });
        register_infix_fn(token::MINUS,     [this](ast::expression* a) -> ast::expression* { return this->parse_infix_expr(a); });
        register_infix_fn(token::SLASH,     [this](ast::expression* a) -> ast::expression* { return this->parse_infix_expr(a); });
        register_infix_fn(token::ASTERISK,  [this](ast::expression* a) -> ast::expression* { return this->parse_infix_expr(a); });
        register_infix_fn(token::EQ,        [this](ast::expression* a) -> ast::expression* { return this->parse_infix_expr(a); });
        register_infix_fn(token::NEQ,       [this](ast::expression* a) -> ast::expression* { return this->parse_infix_expr(a); });
        register_infix_fn(token::LT,        [this](ast::expression* a) -> ast::expression* { return this->parse_infix_expr(a); });
        register_infix_fn(token::GT,        [this](ast::expression* a) -> ast::expression* { return this->parse_infix_expr(a); });
    }

    /**
     * NOTE: Caller is responsible for management of ast::program*
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
        case token::RETURN:
            return parse_return_statement();
            break;
        default:
            return parse_expr_statement();
        }
    }

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

    ast::return_statement* parse_return_statement() noexcept {
        ast::return_statement* stmt = new ast::return_statement(_cur_token);

        next_token();

        while(!cur_token_is(token::SEMICOLON)){
            next_token();
        }

        return stmt;
    }

    ast::expression_statement* parse_expr_statement() noexcept {
        ast::expression_statement* stmt = new ast::expression_statement(_cur_token);
         
        stmt->move_expr(parse_expr(LOWEST));
        
        if(peek_token_is(token::SEMICOLON)) {
            next_token();
        }

        return stmt;
    }

    ast::expression* parse_expr(precedence p) noexcept {
        prefix_parse_fn_t prefix_fn = _prefix_parse_fn_map[_cur_token.get_type()];
        
        if(prefix_fn == nullptr) {
            _errors.push_back("no prefix parse function found for "+token::inv_map[_cur_token.get_type()]);
            return nullptr;
        }

        ast::expression* left_expr = prefix_fn();

        while(!peek_token_is(token::SEMICOLON) && p < peek_precedence()) {
            infix_parse_fn_t infix_fn = _infix_parse_fn_map[_peek_token.get_type()];
            if(prefix_fn == nullptr) {
                return left_expr;
            }
            next_token();
            left_expr = infix_fn(left_expr);
        }
        return left_expr;
    }

    ast::expression* parse_identifier() const noexcept {
        ast::expression* ident = new ast::identifier(_cur_token, _cur_token.token_literal());
        return ident;
    }

    ast::expression* parse_int_literal() const {
        std::int64_t val = std::stoll(std::string(_cur_token.token_literal()));
        ast::expression* lit = new ast::int_literal(_cur_token, val); 
        return lit;
    }

    ast::expression* parse_prefix_expr() noexcept {
        ast::prefix_expression* expr = new ast::prefix_expression(_cur_token, _cur_token.token_literal());
        next_token();
        expr->set_expr(parse_expr(PREFIX));
        return expr;
    }

    bool cur_token_is(token::token_t token_type) const noexcept { return _cur_token.get_type() == token_type; }

    bool peek_token_is(token::token_t token_type) const noexcept { return _peek_token.get_type() == token_type; }

    bool expect_peek(token::token_t token_type) noexcept {
        if(peek_token_is(token_type)){
            next_token();
            return true;
        }else{
            peek_error(token_type);
            return false;
        }
    }
 
    std::vector<std::string> errors() const noexcept { return _errors; }

    void peek_error(token::token_t token_type) noexcept {
        std::ostringstream oss;
        oss << "expected next token to be " 
            << token::inv_map[token_type]
            << ", got " 
            << token::inv_map[_peek_token.get_type()]
            << " instead.";
        _errors.push_back(oss.str());
    }

    void register_prefix_fn(token::token_t token_type, prefix_parse_fn_t prefix_parse_fn) noexcept {
        _prefix_parse_fn_map[token_type] = prefix_parse_fn;
    }

    void register_infix_fn(token::token_t token_type, infix_parse_fn_t infix_parse_fn) noexcept {
        _infix_parse_fn_map[token_type] = infix_parse_fn;
    }

    const precedence peek_precedence() const noexcept {
        auto p = precedences.find(_peek_token.get_type());
        if(p != precedences.end()) {
            return p->second;
        }
        return LOWEST;
    }
    
    const precedence cur_precedence() const noexcept {
        auto p = precedences.find(_cur_token.get_type());
        if(p != precedences.end()) {
            return p->second;
        }
        return LOWEST;
    }

    ast::expression* parse_infix_expr(ast::expression* l_expr) noexcept {
        ast::infix_expression* expr = new ast::infix_expression(_cur_token, _cur_token.token_literal(), l_expr);

        precedence cur_p = cur_precedence();
        next_token();
        expr->right_expr(parse_expr(cur_p));
        
        return expr;
    }

protected:
    lexer::lexer                _l;
    std::vector<std::string>    _errors;

    token::token                _cur_token;
    token::token                _peek_token;

    std::array<prefix_parse_fn_t, token::token_count>   _prefix_parse_fn_map;
    std::array<infix_parse_fn_t, token::token_count>    _infix_parse_fn_map;
    
};

} // namespace parser

