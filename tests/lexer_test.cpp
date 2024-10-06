#include <cstdlib>
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

void test_next_token_1() {
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
            std::cout << "test_next_token_1 - expected token: "
                << test_case[i].expected_token
                << ", got: "
                << cur_token._type
                << std::endl;
            exit(EXIT_FAILURE);
        }
        if(std::string_view(test_case[i].expected_literal) != cur_token._literal){
            std::cout << "test_next_token_1 - expected literal: "
                << *test_case[i].expected_literal
                << ", got: "
                << cur_token._literal
                << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    std::cout<<"ok - test_next_token_1()"<<std::endl;
}

void test_next_token_2() {
    const char* input = R"(
    let five = 5;
    let ten = 10;

    let add = fn(x, y) {
        x + y;
    };

    let result = add(five, ten);
    )";
    std::vector<expected> test_case = {
        {token::LET, "let"}, 
        {token::IDENT, "five"}, 
        {token::ASSIGN, "="}, 
        {token::INT, "5"}, 
        {token::SEMICOLON, ";"}, 

        {token::LET, "let"}, 
        {token::IDENT, "ten"}, 
        {token::ASSIGN, "="}, 
        {token::INT, "10"}, 
        {token::SEMICOLON, ";"}, 

        {token::LET, "let"}, 
        {token::IDENT, "add"}, 
        {token::ASSIGN, "="}, 
        {token::FUNCTION, "fn"}, 
        {token::LPAREN, "("}, 
        {token::IDENT, "x"}, 
        {token::COMMA, ","}, 
        {token::IDENT, "y"}, 
        {token::RPAREN, ")"}, 
        {token::LBRACE, "{"}, 
        {token::IDENT, "x"}, 
        {token::PLUS, "+"}, 
        {token::IDENT, "y"}, 
        {token::SEMICOLON, ";"}, 
        {token::RBRACE, "}"}, 
        {token::SEMICOLON, ";"}, 

        {token::LET, "let"}, 
        {token::IDENT, "result"}, 
        {token::ASSIGN, "="}, 
        {token::IDENT, "add"}, 
        {token::LPAREN, "("}, 
        {token::IDENT, "five"}, 
        {token::COMMA, ","}, 
        {token::IDENT, "ten"}, 
        {token::RPAREN, ")"}, 
        {token::SEMICOLON, ";"}, 

        {token::EOFT, ""}
    };

    lexer l(input);
    token::token cur_token;
    for(int i = 0; i < test_case.size(); i++) {
        cur_token = l.next_token();
        if(test_case[i].expected_token != cur_token._type){
            std::cout << "test_next_token_2 - expected token: "
                << test_case[i].expected_token
                << ", got: "
                << cur_token._type
                << std::endl;
            exit(EXIT_FAILURE);
        }
        if(std::string_view(test_case[i].expected_literal) != cur_token._literal){
            std::cout << "test_next_token_2 - expected literal: "
                << *test_case[i].expected_literal
                << ", got: "
                << cur_token._literal
                << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    std::cout<<"ok - test_next_token_2()"<<std::endl;
}
void test_next_token_3() {
    const char* input = R"(
    !-/*5;
    5 < 10 > 5;
    )";
    std::vector<expected> test_case = {
        {token::BANG, "!"}, 
        {token::MINUS, "-"}, 
        {token::SLASH, "/"}, 
        {token::ASTERISK, "*"}, 
        {token::INT, "5"}, 
        {token::SEMICOLON, ";"}, 

        {token::INT, "5"}, 
        {token::LT, "<"}, 
        {token::INT, "10"}, 
        {token::GT, ">"}, 
        {token::INT, "5"}, 
        {token::SEMICOLON, ";"}, 

        {token::EOFT, ""}
    };

    lexer l(input);
    token::token cur_token;
    for(int i = 0; i < test_case.size(); i++) {
        cur_token = l.next_token();
        if(test_case[i].expected_token != cur_token._type){
            std::cout << "test_next_token_3 - expected token: "
                << test_case[i].expected_token
                << ", got: "
                << cur_token._type
                << std::endl;
            exit(EXIT_FAILURE);
        }
        if(std::string_view(test_case[i].expected_literal) != cur_token._literal){
            std::cout << "test_next_token_3 - expected literal: "
                << *test_case[i].expected_literal
                << ", got: "
                << cur_token._literal
                << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    std::cout<<"ok - test_next_token_3()"<<std::endl;
}

void test_next_token_4() {
    const char* input = R"(
    if (5 < 10) {
        return true;
    } else {
        return false;
    }
    10 != 9;
    5 == 5;
    )";
    std::vector<expected> test_case = {
        {token::IF, "if"},
        {token::LPAREN, "("},
        {token::INT, "5"},
        {token::LT, "<"},
        {token::INT, "10"},
        {token::RPAREN, ")"},
        {token::LBRACE, "{"},
        {token::RETURN, "return"},
        {token::TRUE, "true"},
        {token::SEMICOLON, ";"},
        {token::RBRACE, "}"},
        {token::ELSE, "else"},
        {token::LBRACE, "{"},
        {token::RETURN, "return"},
        {token::FALSE, "false"},
        {token::SEMICOLON, ";"},
        {token::RBRACE, "}"},

        {token::INT, "10"},
        {token::NEQ, "!="},
        {token::INT, "9"},
        {token::SEMICOLON, ";"},

        {token::INT, "5"},
        {token::EQ, "=="},
        {token::INT, "5"},
        {token::SEMICOLON, ";"},

        {token::EOFT, ""}
    };

    lexer l(input);
    token::token cur_token;
    for(int i = 0; i < test_case.size(); i++) {
        cur_token = l.next_token();
        if(test_case[i].expected_token != cur_token._type){
            std::cout << "test_next_token_4 - expected token: "
                << test_case[i].expected_token
                << ", got: "
                << cur_token._type
                << std::endl;
            exit(EXIT_FAILURE);
        }
        if(std::string_view(test_case[i].expected_literal) != cur_token._literal){
            std::cout << "test_next_token_4 - expected literal: "
                << *test_case[i].expected_literal
                << ", got: "
                << cur_token._literal
                << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    std::cout<<"ok - test_next_token_4()"<<std::endl;
}

} // namespace lexer


int main(int argc, char* argv[]) {
    std::cout<<"Running lexer_test.cpp..."<<std::endl;

    lexer::test_next_token_1();
    lexer::test_next_token_2();
    lexer::test_next_token_3();
    lexer::test_next_token_4();

    exit(EXIT_SUCCESS);
}

