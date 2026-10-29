
#pragma once

#include <sstream>
#include <vector>
#include <concepts>
#include <cstddef>
#include <unordered_map>
#include <memory>
#include <optional>
#include "token.hpp"

namespace lex
{

    class Lexer
    {
    public:
        Lexer(const std::unordered_map<std::string_view, TokenTypeInfo> &_reserved);

        /// @brief Generates tokens from the contents of a text file.
        /// @param _filename file to be read.
        /// @return TokenOut::status == ParserStatus::OK on success, or other values on failure with an empty TokenOut::tokens.
        TokenOut tokenize(const std::filesystem::path &_filename);

    private:
        std::shared_ptr<FileBuffer> fileBuffer;
        int token_start_line = 1;
        size_t token_start_index = 0;
        size_t cursor = 0;
        FilePosition position;
        const std::unordered_map<std::string_view, TokenTypeInfo> &reserved;

        /// @brief Reads the contents of a file into the parser's internal buffer.
        /// @param _filename file to be read.
        /// @return ParserStatus::OK on success, or other values specifying what went wrong.
        LexerStatus dumpFile(const std::filesystem::path &_filename);

        bool eof() const;
        char get();
        void unget();
        char peek() const;

        void startToken();
        Token endConsumeToken(TokenType _type);
        Token endToken(TokenType _type) const;
        void discardToken();

        std::string_view extractTokenBody() const;

        void reset();

        void skipNoise();

        /// @brief Reads into token from the internal file buffer while a condition is met.
        /// @param _condition a character-check that has to match for each character pushed into token. First non-matching is putback().
        /// @return true when stopping from a non-matching char, false when EOF or error.
        bool readWhile(std::predicate<unsigned char> auto _condition);

        /// @brief Reads one character from the internal file buffer into token if it matches a condition.
        /// @param _condition a character-check that has to match for the character to be pushed into token, otherwise putback().
        /// @return true when the character is valid and is pushed, false when non-matching and was putback() or an error has happened.
        bool readIf(std::predicate<unsigned char> auto _condition);

        /// @brief Reads in a token if the first character matches head_condition, then continues body_condition. Doesn't read if head_condition doesn't match.
        /// @param _head_condition a character-check that has to match for reading to begin. Stops without consuming if it doesn't match.
        /// @param _tail_condition a character-check that has to match for every subsequent character.
        /// @return whether head_condition matched, i.e if anything was consumed.
        bool readCompound(std::predicate<unsigned char> auto _head_condition, std::predicate<unsigned char> auto _tail_condition);

        /// @brief Same as the overload without the type parameter, but also captures the matching token and assigns to it the given type.
        /// @param _head_condition a character-check that has to match for reading to begin. Stops without consuming if it doesn't match.
        /// @param _tail_condition a character-check that has to match for every subsequent character.
        /// @param _type the type to assign to the token if it matched.
        /// @return the matching token, std::nullopt if it doesn't match.
        std::optional<Token> readCompound(std::predicate<unsigned char> auto _head_condition, std::predicate<unsigned char> auto _tail_condition, TokenType _type);

        size_t remaining() const;
        bool hasRemaining(size_t _length) const;
        std::string_view peek(size_t _length) const;
        std::string_view get(size_t _length);
        bool expect(const std::string_view _toFind);

        std::optional<Token> readIdentifier();

        std::optional<Token> readOperator();

        std::optional<Token> readDelimiter();

        std::optional<Token> readSurroundedLiteral(unsigned char _prefix, unsigned char _suffix, unsigned char _escape, LiteralType _type);

        std::optional<Token> readStringLiteral();
        std::optional<Token> readCharLiteral();
        std::optional<Token> readNumericLiteral();
        std::optional<Token> readBooleanLiteral();
        std::optional<Token> readLiteral();
    };
} // namespace lex