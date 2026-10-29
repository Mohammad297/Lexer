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

lex::Lexer::Lexer(const std::unordered_map<std::string_view, TokenTypeInfo> &_reserved)
    : reserved(_reserved)
{
}

lex::TokenOut lex::Lexer::tokenize(const std::filesystem::path &_filename)
{
    TokenOut out;
    out.status = dumpFile(_filename);

    if (out.status != LexerStatus::OK)
        return out;

    position.path = &_filename;
    position.line = 1;

    out.tokens.reserve(TOKEN_COUNT_ESTIMATE);

    while (!eof())
    {
        skipNoise();
        if (eof())
            break;

        if (auto identifier = readIdentifier())
        {
            std::string_view body = extractTokenBody();
            if (auto it = reserved.find(body); it != reserved.end())
                out.tokens.push_back(endToken(it->second.type));
            else
                out.tokens.push_back(endToken(IdentifierType::OTHER));
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
            startToken();
            readWhile(checkers::isNotWhitespace);
            out.tokens.push_back(endToken(UnknownType::UNKNOWN));
        }
    }

    out.tokens.push_back({EOFType::_EOF, fileBuffer->content.substr(fileBuffer->content.size() - 1, 0), position});

    out.file = std::move(fileBuffer);
    return out;
}

lex::LexerStatus lex::Lexer::dumpFile(const std::filesystem::path &_filename)
{
    std::ifstream file(_filename, std::ios::binary | std::ios::ate);

    if (!file.is_open())
        return LexerStatus::FAILED_OPEN_FILE;

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    fileBuffer = std::make_shared<FileBuffer>();
    fileBuffer->filename = _filename;
    fileBuffer->content.assign(size + 1, '\0'); // + 1 for the sentinel

    if (!file.read(&fileBuffer->content[0], size))
    {
        return LexerStatus::FAILED_BUFFER_READ;
    }

    return LexerStatus::OK;
}

bool lex::Lexer::eof() const
{
    return fileBuffer->content[cursor] == '\0';
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

lex::Token lex::Lexer::endConsumeToken(TokenType _type)
{
    get();
    return endToken(_type);
}

lex::Token lex::Lexer::endToken(TokenType _type) const
{
    return {
        _type,
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

bool lex::Lexer::readWhile(std::predicate<unsigned char> auto _condition)
{
    while (!eof())
    {
        unsigned char c = static_cast<unsigned char>(peek());
        if (_condition(c))
            get();
        else
            return true;
    }
    return false;
}

bool lex::Lexer::readIf(std::predicate<unsigned char> auto _condition)
{
    if (eof())
        return false;

    unsigned char c = static_cast<unsigned char>(peek());
    if (_condition(c))
    {
        get();
        return true;
    }
    return false;
}

bool lex::Lexer::readCompound(std::predicate<unsigned char> auto _head_condition, std::predicate<unsigned char> auto _tail_condition)
{
    if (eof() || !_head_condition(peek()))
        return false;

    get();
    readWhile(_tail_condition);
    return true;
}

std::optional<lex::Token> lex::Lexer::readCompound(std::predicate<unsigned char> auto _head_condition, std::predicate<unsigned char> auto _tail_condition, TokenType _type)
{
    if (eof() || !_head_condition(peek()))
        return std::nullopt;

    startToken();
    get();
    readWhile(_tail_condition);
    return endToken(_type);
}

size_t lex::Lexer::remaining() const
{
    return fileBuffer->content.size() - cursor;
}

bool lex::Lexer::hasRemaining(size_t _length) const
{
    return remaining() >= _length;
}

std::string_view lex::Lexer::peek(size_t _length) const
{
    if (hasRemaining(_length))
        return std::string_view(fileBuffer->content).substr(cursor, _length);
    return std::string_view{};
}

std::string_view lex::Lexer::get(size_t _length)
{
    std::string_view span = peek(_length);
    if (!span.empty())
        cursor += _length;
    return span;
}

bool lex::Lexer::expect(const std::string_view _toFind)
{
    const std::string_view next = get(_toFind.length());
    if (next.empty())
        return false;
    return next == _toFind;
}

std::optional<lex::Token> lex::Lexer::readIdentifier()
{
    return readCompound(checkers::isIdentifierHeader, checkers::isIdentifierBody, IdentifierType::ANY);
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

    static constexpr DelimiterType index_to_enum[] = {
        DelimiterType::LEFT_BRACE,       
        DelimiterType::RIGHT_BRACE,      
        DelimiterType::LEFT_BRACKET,     
        DelimiterType::RIGHT_BRACKET,    
        DelimiterType::LEFT_PARENTHESIS, 
        DelimiterType::RIGHT_PARENTHESIS,
        DelimiterType::PERIOD,           
        DelimiterType::COMMA,            
        DelimiterType::COLON,            
        DelimiterType::SEMICOLON         
    };

    size_t pos = operators.find(peek());
    if (pos == std::string_view::npos)
        return std::nullopt;

    startToken();
    get();

    // Map the position cleanly across the stable lookup buffer in a single cycle
    return endToken(index_to_enum[pos]);
}

std::optional<lex::Token> lex::Lexer::readSurroundedLiteral(unsigned char _prefix, unsigned char _suffix, unsigned char _escape, LiteralType _type)
{
    if (peek() != _prefix)
        return std::nullopt;

    startToken();
    get();
    while (!eof() && peek() != _suffix)
    {
        if (get() == _escape)
            get();
    }
    if (peek() == _suffix)
        return endConsumeToken(_type);
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
    if (!checkers::isDigit(peek()) && peek() != '+' && peek() != '-')
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

std::optional<lex::Token> lex::Lexer::readLiteral()
{
    auto c = readCharLiteral();
    if (c)
        return c;
    auto s = readStringLiteral();
    if (s)
        return s;
    auto n = readNumericLiteral();
    if (n)
        return n;

    return std::nullopt;
}
