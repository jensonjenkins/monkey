#include <iostream>
#include <vector>
#include "../src/lexer.hpp"

namespace lexer {

struct expected {
    token::token_t expected_token;
    const char* expected_literal;

    expected(token::token_t exp_token, const char* exp_lit) 
        : expected_token(exp_token), expected_literal(exp_lit) {};
};

void test_next_token() {
    const char* input = "=+(){},;";
    std::vector<expected> test_case = {
        {token::ASSIGN, "="}, 
        {token::PLUS, "+"}, 
        {token::LPAREN, "("}, 
        {token::RPAREN, ")"}, 
        {token::LBRACE, "{"}, 
        {token::RBRACE, "}"}, 
        {token::COMMA, ","}, 
        {token::SEMICOLON, ";"}, 
        {token::EOFT, ""}
    };

    lexer l(input);
    token::token cur_token;
    for(int i = 0; i < test_case.size(); i++) {
        cur_token = l.next_token();
        if(test_case[i].expected_token != cur_token._type){
            std::cout << "expected token: "
                << test_case[i].expected_token
                << ", got: "
                << cur_token._type
                << std::endl;
        }
        if(*test_case[i].expected_literal != *cur_token._literal){
            std::cout << "expected literal: "
                << *test_case[i].expected_literal
                << ", got: "
                << *cur_token._literal
                << std::endl;
        }
    }
    std::cout<<"ok - test_next_token()"<<std::endl;
}

} // namespace lexer


int main(){
    std::cout<<"Running lexer_test.cpp..."<<std::endl;

    lexer::test_next_token();

    return 0;
}

