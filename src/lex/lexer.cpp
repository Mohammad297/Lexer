#include "lexer.hpp"

#include <fstream>
#include <iostream>
#include <cctype>

#include "checkers.hpp"

static constexpr size_t LINE_COUNT_ESTIMATE = 500;
static constexpr size_t TOKEN_PER_LINE_ESTIMATE = 10;
static constexpr size_t TOKEN_COUNT_ESTIMATE = LINE_COUNT_ESTIMATE * TOKEN_PER_LINE_ESTIMATE;
static constexpr char COMMENT_DELIMITER = '#';
static constexpr char COMMENT_TAIL = '-';

lex::Lexer::Lexer(const std::unordered_map<std::string_view, TokenTypeInfo> &reserved)
    : reserved(reserved)
{
}

lex::TokenOut lex::Lexer::tokenize(const std::string_view filename)
{
    TokenOut out;
    out.status = dumpFile(filename);

    if (out.status != LexerStatus::OK)
        return out;

    out.tokens.reserve(TOKEN_COUNT_ESTIMATE);

    std::cout << "Unknown tokens: [";
    while (!eof())
    {
        skipNoise();
        if (eof())
            break;

        if (auto identifier = readCompound(checkers::isIdentifierHeader, checkers::isIdentifierBody, IdentifierType::ANY))
        {
            std::string_view body = extractTokenBody();
            if (auto it = reserved.find(body); it != reserved.end())
                out.tokens.push_back(endToken(it->second.type));
            else
                out.tokens.push_back(endToken(IdentifierType::ANY));
        }
        else if (auto lit = readLiteral())
        {
            out.tokens.push_back(*lit);
        }
        else if (auto op = readOperator())
        {
            out.tokens.push_back(*op);
        }
        else if (auto delim = readDelimiter())
        {
            out.tokens.push_back(*delim);
        }
        else
        {
            std::cout << get();
        }
    }
    std::cout << ']' << std::endl;

    out.tokens.push_back({EOFType::_EOF, fileBuffer->content.substr(fileBuffer->content.size() - 1, 0), position});

    out.file = std::move(fileBuffer);
    return out;
}

lex::LexerStatus lex::Lexer::dumpFile(const std::string_view filename)
{
    std::ifstream file(std::string(filename), std::ios::binary | std::ios::ate);

    if (!file.is_open())
        return LexerStatus::FAILED_OPEN_FILE;

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    fileBuffer = std::make_shared<FileBuffer>();
    fileBuffer->filename = filename;
    fileBuffer->content.assign(size + 1, '\0'); // + 1 for the sentinel

    if (!file.read(&fileBuffer->content[0], size))
    {
        return LexerStatus::FAILED_BUFFER_READ;
    }

    return LexerStatus::OK;
}

bool lex::Lexer::eof() const
{
    return cursor >= fileBuffer->content.size();
}

char lex::Lexer::get()
{
    if (eof())
        return EOF;

    if (fileBuffer->content[cursor] == '\n')
        position.line++;

    return fileBuffer->content[cursor++];
}

void lex::Lexer::unget()
{
    if (cursor > 0)
    {
        cursor--;
        if (fileBuffer->content[cursor] == '\n')
            position.line--;
    }
}

char lex::Lexer::peek() const
{
    if (!eof())
        return fileBuffer->content[cursor];
    return EOF;
}

void lex::Lexer::startToken()
{
    token_start_line = position.line;
    token_start_index = cursor;
}

lex::Token lex::Lexer::endConsumeToken(TokenType type)
{
    get();
    return endToken(type);
}

lex::Token lex::Lexer::endToken(TokenType type) const
{
    return {
        type,
        extractTokenBody(),
        {position.path, token_start_line}};
}

void lex::Lexer::discardToken()
{
    position.line = token_start_line;
    cursor = token_start_index;
}

std::string_view lex::Lexer::extractTokenBody() const
{
    return std::string_view(fileBuffer->content).substr(token_start_index, cursor - token_start_index);
}

void lex::Lexer::reset()
{
    position.line = 1;
    token_start_line = 1;
    cursor = 0;
    token_start_index = 0;
}

void lex::Lexer::skipNoise()
{
    while (!eof())
    {
        unsigned char c = static_cast<unsigned char>(peek());

        if (std::isspace(c))
            get();
        else if (c == COMMENT_DELIMITER)
        {
            get();
            if (peek() == COMMENT_TAIL)
            {
                while (!eof())
                {
                    if (get() == COMMENT_TAIL && !eof() && peek() == COMMENT_DELIMITER)
                    {
                        get(); // Consumes the final '#'
                        break;
                    }
                }
            }
            else
            {
                while (!eof() && peek() != '\n')
                    get();
            }
        }
        else
            break;
    }
}

bool lex::Lexer::readWhile(std::predicate<unsigned char> auto condition)
{
    while (!eof())
    {
        unsigned char c = static_cast<unsigned char>(peek());
        if (condition(c))
            get();
        else
            return true;
    }
    return false;
}

bool lex::Lexer::readIf(std::predicate<unsigned char> auto condition)
{
    if (eof())
        return false;

    unsigned char c = static_cast<unsigned char>(peek());
    if (condition(c))
    {
        get();
        return true;
    }
}

bool lex::Lexer::readCompound(std::predicate<unsigned char> auto head_condition, std::predicate<unsigned char> auto tail_condition)
{
    if (eof() || !head_condition(peek()))
        return false;

    get();
    readWhile(tail_condition);
    return true;
}

std::optional<lex::Token> lex::Lexer::readCompound(std::predicate<unsigned char> auto head_condition, std::predicate<unsigned char> auto tail_condition, TokenType type)
{
    if (eof() || !head_condition(peek()))
        return std::nullopt;

    startToken();
    get();
    readWhile(tail_condition);
    return endToken(type);
}

size_t lex::Lexer::remaining() const
{
    return fileBuffer->content.size() - cursor;
}

bool lex::Lexer::hasRemaining(size_t length) const
{
    return remaining() >= length;
}

std::string_view lex::Lexer::peek(size_t length) const
{
    if (hasRemaining(length))
        return std::string_view(fileBuffer->content).substr(cursor, length);
    return std::string_view{};
}

std::string_view lex::Lexer::get(size_t length)
{
    std::string_view span = peek(length);
    if (!span.empty())
        cursor += length;
    return span;
}

bool lex::Lexer::expect(const std::string_view toFind)
{
    const std::string_view next = get(toFind.length());
    if (next.empty())
        return false;
    return next == toFind;
}

std::optional<lex::Token> lex::Lexer::readOperator()
{
    unsigned char c = static_cast<unsigned char>(peek());
    switch (c)
    {
    case '+':
        startToken();
        get();
        switch (peek())
        {
        case '+':
            return endConsumeToken(OperatorType::INCREMENT);
        case '=':
            return endConsumeToken(OperatorType::PLUS_ASSIGN);
        }
        return endToken(OperatorType::PLUS);

    case '-':
        startToken();
        get();
        switch (peek())
        {
        case '-':
            return endConsumeToken(OperatorType::DECREMENT);
        case '=':
            return endConsumeToken(OperatorType::MINUS_ASSIGN);
        }
        return endToken(OperatorType::MINUS);

    case '*':
        startToken();
        get();
        switch (peek())
        {
        case '=':
            return endConsumeToken(OperatorType::MULTIPLY_ASSIGN);
        }
        return endToken(OperatorType::MULTIPLY);

    case '/':
        startToken();
        get();
        switch (peek())
        {
        case '=':
            return endConsumeToken(OperatorType::DIVIDE_ASSIGN);
        }
        return endToken(OperatorType::DIVIDE);

    case '%':
        startToken();
        get();
        switch (peek())
        {
        case '=':
            return endConsumeToken(OperatorType::MODULO_ASSIGN);
        }
        return endToken(OperatorType::MODULO);

    case '^':
        startToken();
        get();
        switch (peek())
        {
        case '=':
            return endConsumeToken(OperatorType::EXPONENT_ASSIGN);
        }
        return endToken(OperatorType::EXPONENT);

    case '~':
        startToken();
        get();
        switch (peek())
        {
        case '=':
            return endConsumeToken(OperatorType::BITWISE_NOT_ASSIGN);
        }
        return endToken(OperatorType::BITWISE_NOT);

    case '&':
        startToken();
        get();
        switch (peek())
        {
        case '&':
            get();
            switch (peek())
            {
            case '=':
                return endConsumeToken(OperatorType::LOGIC_AND_ASSIGN);
            }
            return endToken(OperatorType::LOGIC_AND);
        case '=':
            return endConsumeToken(OperatorType::BITWISE_AND_ASSIGN);
        }
        return endToken(OperatorType::BITWISE_AND);

    case '|':
        startToken();
        get();
        switch (peek())
        {
        case '|':
            get();
            switch (peek())
            {
            case '=':
                return endConsumeToken(OperatorType::LOGIC_OR_ASSIGN);
            }
            return endToken(OperatorType::LOGIC_OR);
        case '=':
            return endConsumeToken(OperatorType::BITWISE_OR_ASSIGN);
        }
        return endToken(OperatorType::BITWISE_OR);

    case '!':
        startToken();
        get();
        switch (peek())
        {
        case '=':
            return endConsumeToken(OperatorType::NOT_EQUALS);
        }
        return endToken(OperatorType::LOGIC_NOT);

    case '=':
        startToken();
        get();
        switch (peek())
        {
        case '=':
            return endConsumeToken(OperatorType::EQUALS);
        }
        return endToken(OperatorType::ASSIGN);

    case '<':
        startToken();
        get();
        switch (peek())
        {
        case '<':
            get();
            switch (peek())
            {
            case '=':
                return endConsumeToken(OperatorType::SHIFT_LEFT_ASSIGN);
            }
            return endToken(OperatorType::SHIFT_LEFT);
        case '=':
            return endConsumeToken(OperatorType::LESS_THAN_OR_EQUALS);
        }
        return endToken(OperatorType::LESS_THAN);

    case '>':
        startToken();
        get();
        switch (peek())
        {
        case '>':
            get();
            switch (peek())
            {
            case '=':
                return endConsumeToken(OperatorType::SHIFT_RIGHT_ASSIGN);
            }
            return endToken(OperatorType::SHIFT_RIGHT);
        case '=':
            return endConsumeToken(OperatorType::GREATER_THAN_OR_EQUALS);
        }
        return endToken(OperatorType::GREATER_THAN);

    default:
        return std::nullopt;
    }
}

std::optional<lex::Token> lex::Lexer::readDelimiter()
{
    static constexpr std::string_view operators = "{}[]().,:;";
    size_t pos = operators.find(peek());
    if (pos == std::string_view::npos)
        return std::nullopt;
    startToken();
    get();
    return endToken(static_cast<DelimiterType>(pos));
}

std::optional<lex::Token> lex::Lexer::readSurroundedLiteral(unsigned char prefix, unsigned char suffix, unsigned char escape, LiteralType type)
{
    if (peek() != prefix)
        return std::nullopt;

    startToken();
    get();
    while (!eof() && peek() != suffix)
    {
        if (get() == escape)
            get();
    }
    if (peek() == suffix)
        return endConsumeToken(type);
    discardToken();
    return std::nullopt;
}

std::optional<lex::Token> lex::Lexer::readStringLiteral()
{
    return readSurroundedLiteral('\"', '\"', '\\', LiteralType::STRING);
}

std::optional<lex::Token> lex::Lexer::readCharLiteral()
{
    return readSurroundedLiteral('\'', '\'', '\\', LiteralType::CHAR);
}

std::optional<lex::Token> lex::Lexer::readNumericLiteral()
{
    if (!std::isdigit(peek()) && peek() != '+' && peek() != '-')
        return std::nullopt;

    startToken();
    bool isSigned = (peek() == '+' || peek() == '-');
    if (isSigned)
        get();

    // Must have at least one digit if we consumed a sign
    if (!std::isdigit(peek()))
    {
        discardToken();
        return std::nullopt;
    }

    bool foundDot = false;
    bool foundExp = false;

    // 1. Read Integer part
    while (std::isdigit(peek()))
        get();

    // 2. Optional Fraction part
    if (peek() == '.')
    {
        foundDot = true;
        get();
        while (std::isdigit(peek()))
            get();
    }

    // 3. Optional Exponent part (e.g. 1.2e-10)
    if (peek() == 'e' || peek() == 'E')
    {
        foundExp = true;
        get();
        if (peek() == '+' || peek() == '-')
            get();
        while (std::isdigit(peek()))
            get();
    }

    if (foundDot || foundExp)
        return endToken(LiteralType::FLOAT);
    if (isSigned)
        return endToken(LiteralType::INT);
    return endToken(LiteralType::UINT);
}

std::optional<lex::Token> lex::Lexer::readBooleanLiteral()
{
    startToken();
    if (expect("true") || expect("false"))
        return endToken(LiteralType::BOOL);
    return std::nullopt;
}

std::optional<lex::Token> lex::Lexer::readLiteral()
{
    auto c = readCharLiteral();
    if (c)
        return c;
    auto s = readStringLiteral();
    if (s)
        return s;
    // auto b = readBooleanLiteral();
    // if (b)
    //     return b;
    auto n = readNumericLiteral();
    if (n)
        return n;

    return std::nullopt;
}
