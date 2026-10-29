#include "engine.hpp"

#include "lexer.hpp"

lex::TokenOut lex::Engine::processFile(std::string_view _filename) const
{
    Lexer worker(reserved_vocabulary);
    return worker.tokenize(_filename);
}