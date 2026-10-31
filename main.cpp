//Entrypoint for testing:

#include <iostream>
#include <chrono>
#include <sstream>

#include "lex\token_stream.hpp"
#include "lex\engine.hpp"
#include <fstream>

std::ostream &operator<<(std::ostream &os, const lex::Token &_token)
{
    os << "<" << lex::to_string(_token.type);
    if (!_token.body.empty())
        os << ": '" << _token.body << "'";
    return os << ">";
}

struct TestCase
{
    const std::string filename;
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;
    std::chrono::nanoseconds totalTime;
    size_t tokenCount;
};

std::string formatTime(std::chrono::nanoseconds _time, unsigned int _depth = 2)
{
    std::vector<long long> limits{
        3600'000'000'000 * 24 * 7,
        3600'000'000'000 * 24,
        3600'000'000'000,
        60'000'000'000,
        1000'000'000,
        1000'000,
        1000};

    std::vector<std::string> units{
        "w",
        "d",
        "h",
        "m",
        "s",
        "ms",
        "us",
        "ns"};
    long long nano = _time.count();

    std::stringstream out;

    for (size_t i = 0; i < limits.size(); i++)
    {
        if (_depth == 0)
            break;
        const auto &limit = limits[i];
        const auto &unit = units[i];

        if (nano > limit)
        {
            long long whole = nano / limit;
            out << whole << unit;
            nano = nano % limit;
            _depth--;
        }
    }

    if (_depth > 0)
    {
        out << nano << units.back();
    }

    return out.str();
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "No source file was provided.";
        return 1;
    }

    bool doParseTest = false;
    bool doDumpTokens = false;
    if (argc > 2)
    {
        for (int i = 1; i < argc; i++)
        {
            std::string arg = argv[i];
            if (arg == "-parse")
                doParseTest = true;
            if (arg == "-dump-tokens")
                doDumpTokens = true;
        }
    }

    const std::unordered_map<lex::TokenType, std::vector<std::string_view>> reserved_identifiers = {
        {lex::IdentifierType::KEYWORD, {"if", "while", "else", "switch", "case", "break", "continue", "default", "for", "var", "const", "static"}},
        {lex::IdentifierType::TYPENAME, {"int", "uint", "float", "string", "char", "bool"}},
        {lex::LiteralType::BOOL, {"true", "false"}}};

    std::unordered_map<std::string_view, lex::TokenTypeInfo> reserved;
    for (const auto &[type, set] : reserved_identifiers)
    {
        for (const auto &identifier : set)
        {
            reserved[identifier] = {type};
        }
    }

    std::vector<TestCase> tests;

    lex::Engine engine(reserved.begin(), reserved.end());
    for (int i = 1; i < argc; i++)
    {
        const std::string &arg = argv[i];
        if (arg.starts_with('-'))
            continue;

        const std::string &filename = arg;

        std::cout << "File: " << filename << std::endl;

        auto startTime = std::chrono::high_resolution_clock::now();
        lex::TokenOut out = engine.processFile(filename);

        switch (out.status)
        {
        case lex::LexerStatus::OK:
            break;

        case lex::LexerStatus::FAILED_BUFFER_READ:
            std::cout << "LexerError: Failed to read file buffer." << std::endl;
            continue;

        case lex::LexerStatus::FAILED_OPEN_FILE:
            std::cout << "LexerError: Failed to open file." << std::endl;
            continue;

        default:
            std::cout << "LexerError: Unknown error." << std::endl;
            continue;
        }

        if (doParseTest)
        {
            lex::TokenStream stream(out);

            size_t validStatements = 0;
            size_t totalAttempts = 0;

            std::cout << "Parsing stream grammar..." << std::endl;

            while (stream.peek())
            {
                totalAttempts++;
                size_t rollbackMarker = stream.tell(); // Safe backtracking save-state!

                // Pattern 1: Attempt to parse a Variable Declaration -> var [name] = [literal]
                if (stream.match(lex::IdentifierType::KEYWORD))
                {
                    // We just saw a keyword. Let's strictly expect an identifier name next
                    if (stream.expect(lex::IdentifierType::ANY))
                    {
                        // Next look for an assignment operator '='
                        if (stream.expect(lex::OperatorType::ASSIGN))
                        {
                            // Check if it's assigned to any literal type (INT, FLOAT, STRING, BOOL, CHAR)
                            auto isLiteral = [](const lex::Token &t)
                            {
                                return std::holds_alternative<lex::LiteralType>(t.type);
                            };

                            if (stream.expect(isLiteral))
                            {
                                validStatements++;
                                continue; // Successfully parsed a mock statement! Stream is advanced.
                            }
                        }
                    }
                }

                // Reset the cursor stream back to where we started this loop iteration
                stream.seek(rollbackMarker);

                // Pattern 2: Attempt to parse a Control flow -> if [condition] break;
                if (stream.match(lex::TokenType(lex::IdentifierType::KEYWORD))) // Assumes you check for 'if' text in a real parser
                {
                    if (stream.expect(lex::IdentifierType::ANY))
                    {
                        if (stream.expect(lex::IdentifierType::KEYWORD)) // matches 'break' / 'continue'
                        {
                            validStatements++;
                            continue;
                        }
                    }
                }

                // Fallback: If no patterns matched, consume exactly 1 token to keep marching forward
                stream.seek(rollbackMarker);
                stream.consume();
            }

            std::cout << "\tGrammar Stats: Found " << validStatements
                      << " accidental legal phrases out of " << totalAttempts
                      << " stream sweeps." << std::endl;
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = endTime - startTime;
        tests.push_back({filename, startTime, endTime, duration, out.tokens.size()});

        if (doDumpTokens) {
            std::filesystem::path filepath(filename);
            std::filesystem::path tkdump_path = filepath.replace_filename(std::format("__tkdump__{}", filepath.filename().string()));
            std::ofstream output(tkdump_path);

            if (output.is_open()) {
                std::cout << "dumping tokens to: " << tkdump_path << std::endl;
                size_t consequent_body = 1;
                for (size_t i = 0; i < out.tokens.size(); i++) {
                    if (i > 1 && out.tokens[i].body == out.tokens[i - 1].body)
                        consequent_body++;
                    else if (consequent_body > 1){
                        output << " x" << consequent_body << std::endl << out.tokens[i];
                        consequent_body = 1;
                    }
                    else {
                        output << out.tokens[i] << std::endl;;
                    }
                }
            }
            else {
                std::cout << "failed to dump tokens to: " << tkdump_path << std::endl;
            }
        }
        else {
            std::cout << "Tokens:" << std::endl;
            if (out.tokens.size() < 50)
            {
                for (const auto &token : out.tokens)
                {
                    std::cout << token << " ";
                }
                std::cout << std::endl;
            }
            else
            {
                for (size_t i = 0; i < 10; i++)
                    std::cout << out.tokens[i] << " ";
                std::cout << "... " << std::endl;
                for (size_t i = 0; i < 10; i++)
                    std::cout << out.tokens[out.tokens.size() - 11 + i] << " ";
                std::cout << std::endl;
            }
            std::cout << "Tests:" << std::endl;
            for (const auto &test : tests)
            {
                std::cout << "\t" << test.filename << ": \t" << formatTime(test.totalTime) << " \ttokens: " << test.tokenCount << std::endl;
            }
        }
    }
    return 0;
}