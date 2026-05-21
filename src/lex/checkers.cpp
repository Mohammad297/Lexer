#include "checkers.hpp"

#include <cctype>

bool lex::checkers::isIdentifierHeader(unsigned char c)
{
    return std::isalpha(c) || c == '_';
}

bool lex::checkers::isIdentifierBody(unsigned char c)
{
    return std::isalnum(c) || c == '_';
}

bool lex::checkers::isNotWhitespace(unsigned char c)
{
    return !std::isspace(c);
}

bool lex::checkers::isNotNewline(unsigned char c)
{
    return c != '\n';
}
