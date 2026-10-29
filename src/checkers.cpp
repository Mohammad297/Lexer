#include "checkers.hpp"

#include <cctype>

bool lex::checkers::isDigit(unsigned char _c)
{
    return _c >= '0' && _c <= '9';
}

bool lex::checkers::isIdentifierHeader(unsigned char _c)
{
    return std::isalpha(_c) || _c == '_';
}

bool lex::checkers::isIdentifierBody(unsigned char _c)
{
    return std::isalnum(_c) || _c == '_';
}

bool lex::checkers::isNotWhitespace(unsigned char _c)
{
    return !std::isspace(_c);
}

bool lex::checkers::isNotNewline(unsigned char _c)
{
    return _c != '\n';
}
