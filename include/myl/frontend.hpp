#pragma once

#include "myl/ast.hpp"
#include <memory>
#include <string>
#include <vector>

namespace myl {

// A token as observed by the lexer, used for unit tests and diagnostics.
struct TokenInfo {
    std::string kind;
    std::string text;
    int line = 1;
    int column = 1;
};

// Parse a whole source buffer into a Program.
// Returns nullptr on failure; diagnostics are appended to `errors`.
std::unique_ptr<Program> parseSource(const std::string& source,
                                     const std::string& filename,
                                     std::vector<std::string>& errors);

// Lex a buffer into a flat token list (used by the lexer unit tests).
std::vector<TokenInfo> tokenize(const std::string& source,
                                const std::string& filename,
                                std::vector<std::string>& errors);

} // namespace myl
