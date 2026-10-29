#pragma once

#include "token.hpp"

#include <concepts>
#include <string_view>

namespace lex
{
    class TokenStream
    {
    public:
        TokenStream(const TokenOut &_tokens);
        TokenStream(TokenOut &&) = delete;

        bool eof() const;

        const Token *peek() const;
        const Token *consume();

        const Token* check(std::predicate<Token> auto _condition)   { return check_impl(_condition); }
        const Token* check(TokenType _type)                         { return check_impl(_type); }
        const Token* check(TokenCategory _category)                 { return check_impl(_category); }
        const Token* check(const std::string_view _body)            { return check_impl(_body); }

        const Token *match(std::predicate<Token> auto _condition)   { return match_impl(_condition); }
        const Token *match(TokenType _type)                         { return match_impl(_type); }
        const Token *match(TokenCategory _category)                 { return match_impl(_category); }
        const Token *match(const std::string_view _body)            { return match_impl(_body); }

        const Token *expect(std::predicate<Token> auto _condition)  { return expect_impl(_condition); }
        const Token *expect(TokenType _type)                        { return expect_impl(_type); }
        const Token *expect(TokenCategory _category)                { return expect_impl(_category); }
        const Token *expect(const std::string_view _body)           { return expect_impl(_body); }

        size_t tell() const;
        bool seek(size_t _new_position);

        struct Mark
        {
        private:
            const size_t offset;

            explicit Mark(size_t _position);
            friend class TokenStream;
        };

        [[nodiscard]] Mark mark() const;
        size_t rollback(const Mark &_mark);

    private:
        const TokenOut &tokens;
        size_t cursor;

        const Token* check_predicate(std::predicate<Token> auto _condition) {
            auto current = peek();
            if (current && _condition(*current)) {
                return current;
            }
            return nullptr;
        }

        auto make_predicate(TokenType _type) {
            return [=](const Token& token)
                { return token.type == _type; };
        }
        auto make_predicate(TokenCategory _category) {
            return [=](const Token& token)
                { return getCategory(token.type) == _category; };
        }
        auto make_predicate(const std::string_view _body) {
            return [=](const Token& token)
                { return token.body == _body; };
        }
        auto make_predicate(std::predicate<Token> auto _condition) { 
            return _condition;
        }

        const Token* check_impl(auto arg) {
            return check_predicate(make_predicate(arg));
        }

        const Token* match_impl(auto arg) {
            if (const Token* token = check_impl(arg)) {
                cursor++;
                return token;
            }
            return nullptr;
        }

        const Token* expect_impl(auto arg) {
            if (const Token* token = match_impl(arg)) {
                return token;
            }
            //no throwing for now. Left for the parser.
            return nullptr; 
        }

    };
} // namespace lex
