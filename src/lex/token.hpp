#pragma once

#include <cstdint>
#include <variant>
#include <string_view>
#include <memory>
#include <vector>
#include <string>

namespace lex
{
    enum class LexerStatus
    {
        OK,
        FAILED_OPEN_FILE,
        FAILED_BUFFER_READ
    };

    struct FileBuffer
    {
        std::string filename;
        std::string content;
    };

    enum class TokenCategory : uint8_t
    {
        IDENTIFIER,
        LITERAL,
        OPERATOR,
        DELIMITER,
        _EOF,
    };

    inline std::string_view to_string(TokenCategory category)
    {
        switch (category)
        {
        case TokenCategory::IDENTIFIER:
            return "identifier";
        case TokenCategory::LITERAL:
            return "literal";
        case TokenCategory::OPERATOR:
            return "operator";
        case TokenCategory::DELIMITER:
            return "delimiter";
        case TokenCategory::_EOF:
            return "EOF";
        default:
            return "ERROR";
        }
    }

    enum class IdentifierType : uint8_t
    {
        ANY,
        KEYWORD,
        TYPENAME,
        OTHER
    };

    inline std::string_view to_string(IdentifierType type)
    {
        switch (type)
        {
        case IdentifierType::ANY:
            return "any-identifier";
        case IdentifierType::KEYWORD:
            return "keyword";
        case IdentifierType::OTHER:
            return "other-identifier";
        case IdentifierType::TYPENAME:
            return "typename";
        default:
            return "ERROR";
        }
    }

    enum class LiteralType : uint8_t
    {
        UINT,
        INT,
        FLOAT,
        CHAR,
        STRING,
        BOOL,
    };

    inline std::string_view to_string(LiteralType type)
    {
        switch (type)
        {
        case LiteralType::UINT:
            return "uint-literal";
        case LiteralType::INT:
            return "int-literal";
        case LiteralType::FLOAT:
            return "float-literal";
        case LiteralType::CHAR:
            return "char-literal";
        case LiteralType::STRING:
            return "string-literal";
        case LiteralType::BOOL:
            return "bool-literal";
        default:
            return "ERROR";
        }
    }

    enum class OperatorType : uint8_t
    {
        INCREMENT,
        DECREMENT,
        PLUS,
        MINUS,
        MULTIPLY,
        DIVIDE,
        MODULO,
        EXPONENT,
        SHIFT_RIGHT,
        SHIFT_LEFT,
        LOGIC_AND,
        LOGIC_OR,
        LOGIC_NOT,
        BITWISE_AND,
        BITWISE_OR,
        BITWISE_NOT,
        ASSIGN,
        PLUS_ASSIGN,
        MINUS_ASSIGN,
        MULTIPLY_ASSIGN,
        DIVIDE_ASSIGN,
        MODULO_ASSIGN,
        EXPONENT_ASSIGN,
        LOGIC_AND_ASSIGN,
        LOGIC_OR_ASSIGN,
        SHIFT_RIGHT_ASSIGN,
        SHIFT_LEFT_ASSIGN,
        BITWISE_AND_ASSIGN,
        BITWISE_OR_ASSIGN,
        BITWISE_NOT_ASSIGN,
        EQUALS,
        NOT_EQUALS,
        LESS_THAN,
        GREATER_THAN,
        LESS_THAN_OR_EQUALS,
        GREATER_THAN_OR_EQUALS
    };

    inline std::string_view to_string(OperatorType type)
    {
        switch (type)
        {
        case OperatorType::INCREMENT:
            return "increment";
        case OperatorType::DECREMENT:
            return "decrement";
        case OperatorType::PLUS:
            return "plus";
        case OperatorType::MINUS:
            return "minus";
        case OperatorType::MULTIPLY:
            return "multiply";
        case OperatorType::DIVIDE:
            return "divide";
        case OperatorType::MODULO:
            return "modulo";
        case OperatorType::EXPONENT:
            return "exponent";
        case OperatorType::SHIFT_RIGHT:
            return "shift-right";
        case OperatorType::SHIFT_LEFT:
            return "shift-left";
        case OperatorType::LOGIC_AND:
            return "logic-and";
        case OperatorType::LOGIC_OR:
            return "logic-or";
        case OperatorType::LOGIC_NOT:
            return "logic-not";
        case OperatorType::BITWISE_AND:
            return "bitwise-and";
        case OperatorType::BITWISE_OR:
            return "bitwise-or";
        case OperatorType::BITWISE_NOT:
            return "bitwise-not";
        case OperatorType::ASSIGN:
            return "assign";
        case OperatorType::PLUS_ASSIGN:
            return "plus-assign";
        case OperatorType::MINUS_ASSIGN:
            return "minus-assign";
        case OperatorType::MULTIPLY_ASSIGN:
            return "multiply-assign";
        case OperatorType::DIVIDE_ASSIGN:
            return "divide-assign";
        case OperatorType::MODULO_ASSIGN:
            return "modulo-assin";
        case OperatorType::EXPONENT_ASSIGN:
            return "exponent-assign";
        case OperatorType::LOGIC_AND_ASSIGN:
            return "logic-and-assign";
        case OperatorType::LOGIC_OR_ASSIGN:
            return "logic-or-assign";
        case OperatorType::SHIFT_RIGHT_ASSIGN:
            return "shift-right-assign";
        case OperatorType::SHIFT_LEFT_ASSIGN:
            return "shift-left-assign";
        case OperatorType::BITWISE_AND_ASSIGN:
            return "bitwise-and-assign";
        case OperatorType::BITWISE_OR_ASSIGN:
            return "bitwise-or-assign";
        case OperatorType::BITWISE_NOT_ASSIGN:
            return "bitwise-not-assign";
        case OperatorType::EQUALS:
            return "equals";
        case OperatorType::NOT_EQUALS:
            return "not-equals";
        case OperatorType::LESS_THAN:
            return "less-than";
        case OperatorType::GREATER_THAN:
            return "greater-than";
        case OperatorType::LESS_THAN_OR_EQUALS:
            return "less-than-or-equals";
        case OperatorType::GREATER_THAN_OR_EQUALS:
            return "greater-than-or-equals";
        default:
            return "ERROR";
        }
    }

    enum class DelimiterType : uint8_t
    {
        LEFT_BRACE,
        RIGHT_BRACE,
        LEFT_BRACKET,
        RIGHT_BRACKET,
        LEFT_PARENTHESIS,
        RIGHT_PARENTHESIS,
        PERIOD,
        COMMA,
        COLON,
        SEMICOLON
    };

    inline std::string_view to_string(DelimiterType type)
    {
        switch (type)
        {
        case DelimiterType::LEFT_BRACE:
            return "left-brace";
        case DelimiterType::RIGHT_BRACE:
            return "right-brace";
        case DelimiterType::LEFT_BRACKET:
            return "left-bracket";
        case DelimiterType::RIGHT_BRACKET:
            return "right-bracket";
        case DelimiterType::LEFT_PARENTHESIS:
            return "left-parenthesis";
        case DelimiterType::RIGHT_PARENTHESIS:
            return "right-parenthesis";
        case DelimiterType::PERIOD:
            return "period";
        case DelimiterType::COMMA:
            return "comma";
        case DelimiterType::COLON:
            return "colon";
        case DelimiterType::SEMICOLON:
            return "semicolon";
        default:
            return "ERROR";
        }
    }

    enum class EOFType : uint8_t
    {
        _EOF
    };

    inline std::string_view to_string(EOFType type)
    {
        switch (type)
        {
        case EOFType::_EOF:
            return "EOF";
        default:
            return "ERROR";
        }
    }

    using TokenType = std::variant<IdentifierType, LiteralType, OperatorType, DelimiterType, EOFType>;

    inline std::string_view to_string(const TokenType &type)
    {
        return std::visit([](auto &&arg)
                          { return to_string(arg); }, type);
    }

    struct TokenTypeInfo
    {
        TokenType type;
    };

    struct FilePosition
    {
        const std::string_view path;
        int line = 1;
    };

    struct Token
    {
        TokenType type;
        const std::string_view body;
        FilePosition position;
    };

    struct TokenOut
    {
        LexerStatus status;
        std::shared_ptr<FileBuffer> file;
        std::vector<Token> tokens;
    };
} // namespace lex