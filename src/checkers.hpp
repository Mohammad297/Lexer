#pragma once

namespace lex
{
    namespace checkers
    {
        bool isDigit(unsigned char _c);
        bool isIdentifierHeader(unsigned char _c);
        bool isIdentifierBody(unsigned char _c);
        bool isNotWhitespace(unsigned char _c);
        bool isNotNewline(unsigned char _c);
    } // namespace checkers
} // namespace lex
