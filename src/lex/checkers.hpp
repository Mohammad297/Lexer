#pragma once

namespace lex
{
    namespace checkers
    {
        bool isIdentifierHeader(unsigned char c);
        bool isIdentifierBody(unsigned char c);
        bool isNotWhitespace(unsigned char c);
        bool isNotNewline(unsigned char c);
    } // namespace checkers
} // namespace lex
