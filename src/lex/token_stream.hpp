#pragma once

#include "token.hpp"

#include <optional>
#include <concepts>

namespace lex
{
    class TokenStream
    {
    public:
        TokenStream(const TokenOut &_tokens);
        TokenStream(TokenOut &&) = delete;

        const std::optional<Token> peek() const;
        const std::optional<Token> match(std::predicate<Token> auto condition);
        const std::optional<Token> match(TokenType type);
        const std::optional<Token> match(const std::string_view body);
        const std::optional<Token> consume();
        const std::optional<Token> expect(std::predicate<Token> auto condition);
        const std::optional<Token> expect(TokenType type);
        const std::optional<Token> expect(const std::string_view body);
        size_t tell() const;
        bool seek(size_t new_position);

        struct Mark
        {
        private:
            const size_t offset;

            explicit Mark(size_t _position);
            friend class TokenStream;
        };

        [[nodiscard]] Mark mark() const;
        size_t rollback(const Mark &mark);

    private:
        const TokenOut &tokens;
        size_t cursor;
    };

    const std::optional<lex::Token> TokenStream::match(std::predicate<Token> auto condition)
    {
        auto current = peek();
        if (current && condition(*current))
        {
            cursor++;
            return current;
        }
        return std::nullopt;
    }

    const std::optional<Token> TokenStream::expect(std::predicate<Token> auto condition)
    {
        return match(condition);
    }
} // namespace lex
