#pragma once

#include "token.hpp"

namespace parser {

template <token::token_t T>
void prefix_parse_fn();

template <token::token_t T>
void infix_parse_fn();

template <>
void prefix_parse_fn<token::IDENT>();

template <>
void infix_parse_fn<token::IDENT>();

} // namespace parser
