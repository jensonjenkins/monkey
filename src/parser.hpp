#pragma once

#include "token.hpp"
#include "ast.hpp"
#include "lexer.hpp"
#include "trace.hpp"
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
    {token::LPAREN,     CALL},
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
        register_prefix_fn(token::TRUE,     [this]() -> ast::expression* { return this->parse_boolean(); });
        register_prefix_fn(token::FALSE,    [this]() -> ast::expression* { return this->parse_boolean(); });
        register_prefix_fn(token::LPAREN,   [this]() -> ast::expression* { return this->parse_grouped_expr(); });
        register_prefix_fn(token::IF,       [this]() -> ast::expression* { return this->parse_if_expression(); });
        register_prefix_fn(token::FUNCTION, [this]() -> ast::expression* { return this->parse_function_literal(); });
        register_prefix_fn(token::STRING,   [this]() -> ast::expression* { return this->parse_string_literal(); });

        register_infix_fn(token::PLUS,      [this](ast::expression* a) -> ast::expression* { return this->parse_infix_expr(a); });
        register_infix_fn(token::MINUS,     [this](ast::expression* a) -> ast::expression* { return this->parse_infix_expr(a); });
        register_infix_fn(token::SLASH,     [this](ast::expression* a) -> ast::expression* { return this->parse_infix_expr(a); });
        register_infix_fn(token::ASTERISK,  [this](ast::expression* a) -> ast::expression* { return this->parse_infix_expr(a); });
        register_infix_fn(token::EQ,        [this](ast::expression* a) -> ast::expression* { return this->parse_infix_expr(a); });
        register_infix_fn(token::NEQ,       [this](ast::expression* a) -> ast::expression* { return this->parse_infix_expr(a); });
        register_infix_fn(token::LT,        [this](ast::expression* a) -> ast::expression* { return this->parse_infix_expr(a); });
        register_infix_fn(token::GT,        [this](ast::expression* a) -> ast::expression* { return this->parse_infix_expr(a); });
        register_infix_fn(token::LPAREN,    [this](ast::expression* a) -> ast::expression* { return this->parse_call_expr(a); });
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
        trace t("parse_let_stmt: " + std::string(_cur_token.token_literal()));
        ast::let_statement* stmt = new ast::let_statement(_cur_token);

        if(!expect_peek(token::IDENT)){
            return nullptr;
        }

        stmt->move_ident(ast::identifier(_cur_token, _cur_token.token_literal()));
        if(!expect_peek(token::ASSIGN)){
            return nullptr;
        }
        next_token();

        stmt->set_value(parse_expr(LOWEST));
        if(peek_token_is(token::SEMICOLON)) {
            next_token();
        }
        return stmt;
    }

    ast::return_statement* parse_return_statement() noexcept {
        trace t("parse_return_stmt: " + std::string(_cur_token.token_literal()));
        ast::return_statement* stmt = new ast::return_statement(_cur_token);

        next_token();

        stmt->set_return_value(parse_expr(LOWEST));
        if(peek_token_is(token::SEMICOLON)) {
            next_token();
        }

        return stmt;
    }

    ast::expression_statement* parse_expr_statement() noexcept {
        trace t("parse_expr_statement: " + std::string(_cur_token.token_literal()));
        ast::expression_statement* stmt = new ast::expression_statement(_cur_token);
         
        stmt->move_expr(parse_expr(LOWEST));
        
        if(peek_token_is(token::SEMICOLON)) {
            next_token();
        }

        return stmt;
    }

    ast::expression* parse_expr(precedence p) noexcept {
        trace t("parse_expr: " + std::string(_cur_token.token_literal()));
        prefix_parse_fn_t prefix_fn = _prefix_parse_fn_map[_cur_token.get_type()];
        
        if(prefix_fn == nullptr) {
            _errors.push_back("no prefix parse function found for "+token::inv_map[_cur_token.get_type()]);
            return nullptr;
        }

        ast::expression* left_expr = prefix_fn();

        while(!peek_token_is(token::SEMICOLON) && p < peek_precedence()) {
            infix_parse_fn_t infix_fn = _infix_parse_fn_map[_peek_token.get_type()];
            if(infix_fn == nullptr) {
                return left_expr;
            }
            next_token();
            left_expr = infix_fn(left_expr);
        }
        return left_expr;
    }

    ast::expression* parse_identifier() const noexcept {
        trace t("parse_ident: " + std::string(_cur_token.token_literal()));
        ast::expression* ident = new ast::identifier(_cur_token, _cur_token.token_literal());
        return ident;
    }

    ast::expression* parse_boolean() const noexcept {
        trace t("parse_boolean: " + std::string(_cur_token.token_literal()));
        ast::expression* boolean = new ast::boolean(_cur_token, cur_token_is(token::TRUE));
        return boolean;
    }

    ast::expression* parse_int_literal() const {
        trace t("parse_int_literal: " + std::string(_cur_token.token_literal()));
        std::int64_t val = std::stoll(std::string(_cur_token.token_literal()));
        ast::expression* lit = new ast::int_literal(_cur_token, val); 
        return lit;
    }

    ast::expression* parse_string_literal() const noexcept {
        trace t("parse_str_literal: " + std::string(_cur_token.token_literal()));
        ast::expression* string_lit = new ast::string_literal(_cur_token, _cur_token.token_literal());
        return string_lit;
    }

    ast::expression* parse_prefix_expr() noexcept {
        trace t("parse_prefix_expr: " + std::string(_cur_token.token_literal()));
        ast::prefix_expression* expr = new ast::prefix_expression(_cur_token, _cur_token.token_literal());
        next_token();
        expr->set_expr(parse_expr(PREFIX));
        return expr;
    }

    ast::expression* parse_grouped_expr() noexcept {
        trace t("parse_grouped_expr: " + std::string(_cur_token.token_literal()));
        next_token();
        ast::expression* exp = parse_expr(LOWEST);

        if(!expect_peek(token::RPAREN)) {
            return nullptr;
        }
        return exp;
    }

    ast::expression* parse_if_expression() noexcept {
        trace t("parse_if_expr: " + std::string(_cur_token.token_literal()));
        ast::if_expression* expr = new ast::if_expression(_cur_token);

        if(!expect_peek(token::LPAREN)){
            return nullptr;
        }
        next_token(); 
        expr->set_condition(parse_expr(LOWEST));

        if(!expect_peek(token::RPAREN)) {
            return nullptr;
        }
        if(!expect_peek(token::LBRACE)) {
            return nullptr;
        }
        expr->set_consequence(parse_block_statement());

        if(peek_token_is(token::ELSE)) {
            next_token();
            if(!expect_peek(token::LBRACE)){
                return nullptr;
            }
            expr->set_alternative(parse_block_statement());
        }

        return expr;
    } 

    ast::block_statement* parse_block_statement() noexcept {
        trace t("parse_block_statement: " + std::string(_cur_token.token_literal()));
        ast::block_statement* block = new ast::block_statement();
        next_token();

        while(!cur_token_is(token::RBRACE) && !cur_token_is(token::EOFT)) {
            ast::statement* stmt = parse_statement();
            if(stmt != nullptr) {
                block->add_statement(stmt);
            }
            next_token();
        }
        return block;
    }

    ast::expression* parse_function_literal() noexcept {
        trace t("parse_fn_literal: " + std::string(_cur_token.token_literal()));
        ast::function_literal* fn = new ast::function_literal();

        if(!expect_peek(token::LPAREN)){
            return nullptr;
        }

        fn->set_parameters(parse_function_parameters());

        if(!expect_peek(token::LBRACE)){
            return nullptr;
        }

        fn->set_body(parse_block_statement());

        return fn;
    }

    std::vector<ast::identifier*> parse_function_parameters() noexcept {
        trace t("parse_fn_parameters: " + std::string(_cur_token.token_literal()));
        std::vector<ast::identifier*> identifiers;
        if(peek_token_is(token::RPAREN)) {
            next_token();
            return identifiers;
        }
        next_token();

        ast::identifier* ident = new ast::identifier(_cur_token, _cur_token.token_literal());
        identifiers.push_back(ident);

        while(peek_token_is(token::COMMA)) {
            next_token();
            next_token();
            ast::identifier* ident = new ast::identifier(_cur_token, _cur_token.token_literal());
            identifiers.push_back(ident);
        }

        if(!expect_peek(token::RPAREN)) {
            std::vector<ast::identifier*> nil;
            return nil;
        }

        return identifiers;
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
        trace t("parse_infix_expr: " + std::string(_cur_token.token_literal()));
        ast::infix_expression* expr = new ast::infix_expression(_cur_token, _cur_token.token_literal(), l_expr);

        precedence cur_p = cur_precedence();
        next_token();
        expr->right_expr(parse_expr(cur_p));
        
        return expr;
    }

    ast::expression* parse_call_expr(ast::expression* function) noexcept {
        trace t("parse_call_expr: " + std::string(_cur_token.token_literal()));
        ast::call_expression* expr = new ast::call_expression(_cur_token, function);
        expr->set_arguments(parse_call_args());
        return expr;
    }

    std::vector<ast::expression*> parse_call_args() noexcept {
        trace t("parse_call_args: " + std::string(_cur_token.token_literal()));
        std::vector<ast::expression*> args;

        if(peek_token_is(token::RPAREN)){
            next_token();
            return args;
        }

        next_token();
        args.push_back(parse_expr(LOWEST));

        while(peek_token_is(token::COMMA)) {
            next_token();
            next_token();
            args.push_back(parse_expr(LOWEST));
        }

        if(!expect_peek(token::RPAREN)) {
            std::vector<ast::expression*> nil;
            return nil;
        }

        return args;
    }

protected:
    lexer::lexer                _l;
    token::token                _cur_token;
    token::token                _peek_token;
    std::vector<std::string>    _errors;

    std::array<prefix_parse_fn_t, token::token_count>   _prefix_parse_fn_map;
    std::array<infix_parse_fn_t, token::token_count>    _infix_parse_fn_map;
    
};

} // namespace parser

