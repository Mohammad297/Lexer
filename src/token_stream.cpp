#include "token_stream.hpp"

lex::TokenStream::TokenStream(const TokenOut &_tokens)
    : tokens(_tokens), cursor(0)
{
}

const lex::Token *lex::TokenStream::peek() const
{
    const Token &current = tokens.tokens[cursor];
    return std::holds_alternative<EOFType>(current.type) ? nullptr : &current;
}

const lex::Token *lex::TokenStream::consume()
{
    return expect([](const Token &)
                  { return true; });
}

bool lex::TokenStream::seek(size_t _new_position)
{
    if (_new_position <= tokens.tokens.size())
    {
        cursor = _new_position;
        return true;
    }
    return false;
}

bool lex::TokenStream::eof() const
{
    return peek() == nullptr;
}

lex::TokenStream::Mark lex::TokenStream::mark() const
{
    return Mark{tell()};
}

size_t lex::TokenStream::rollback(const Mark &_mark)
{
    if (_mark.offset > tell())
        return 0;

    size_t distance = tell() - _mark.offset;

    seek(_mark.offset);
    return distance;
}

size_t lex::TokenStream::tell() const
{
    return cursor;
}

lex::TokenStream::Mark::Mark(size_t _position)
    : offset(_position)
{
}
