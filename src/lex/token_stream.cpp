#include "token_stream.hpp"

lex::TokenStream::TokenStream(const TokenOut &_tokens)
    : tokens(_tokens), cursor(0)
{
}

const std::optional<lex::Token> lex::TokenStream::peek() const
{
    const Token &current = tokens.tokens[cursor];
    return std::holds_alternative<EOFType>(current.type) ? std::nullopt : std::optional(current);
}

const std::optional<lex::Token> lex::TokenStream::match(TokenType type)
{
    return match([&type](const Token &token)
                 { return token.type == type; });
}

const std::optional<lex::Token> lex::TokenStream::match(const std::string_view body)
{
    return match([&body](const Token &token)
                 { return token.body == body; });
}

const std::optional<lex::Token> lex::TokenStream::consume()
{
    return expect([](const Token &)
                  { return true; });
}

const std::optional<lex::Token> lex::TokenStream::expect(TokenType type)
{
    return expect([&type](const Token &token)
                  { return token.type == type; });
}

const std::optional<lex::Token> lex::TokenStream::expect(const std::string_view body)
{
    return expect([&body](const Token &token)
                  { return token.body == body; });
}

bool lex::TokenStream::seek(size_t new_position)
{
    if (new_position <= tokens.tokens.size())
    {
        cursor = new_position;
        return true;
    }
    return false;
}

lex::TokenStream::Mark lex::TokenStream::mark() const
{
    return Mark{tell()};
}

size_t lex::TokenStream::rollback(const Mark &mark)
{
    if (mark.offset > tell())
        return 0;

    size_t distance = tell() - mark.offset;

    seek(mark.offset);
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
