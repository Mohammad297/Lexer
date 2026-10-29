#pragma once

#include <unordered_map>
#include <string_view>

#include "token.hpp"

namespace lex
{
    class Engine
    {
    public:
        template <typename Iter>
        explicit Engine(Iter _firstReserved, Iter _lastReserved)
            : reserved_vocabulary(_firstReserved, _lastReserved)
        {
        }

        [[nodiscard]] TokenOut processFile(std::string_view _filename) const;

    private:
        std::unordered_map<std::string_view, TokenTypeInfo> reserved_vocabulary;
    };
} // namespace lex
