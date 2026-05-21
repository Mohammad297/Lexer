#include "engine.hpp"

#include "lexer.hpp"

lex::TokenOut lex::Engine::processFile(std::string_view filename) const
{
    Lexer worker(reserved_vocabulary);
    return worker.tokenize(filename);
}