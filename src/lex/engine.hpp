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
        explicit Engine(Iter firstReserved, Iter lastReserved)
            : reserved_vocabulary(firstReserved, lastReserved)
        {
        }

        [[nodiscard]] TokenOut processFile(std::string_view filename) const;

    private:
        std::unordered_map<std::string_view, TokenTypeInfo> reserved_vocabulary;
    };
} // namespace lex
