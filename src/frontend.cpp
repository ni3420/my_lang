#include "myl/frontend.hpp"
#include "parser.hpp"

#include <memory>
#include <string>
#include <vector>

extern "C" void myl_set_input(const char* source, const char* filename);
extern "C" void myl_cleanup_input();
extern std::vector<std::string>* g_parse_errors;

namespace myl {

std::unique_ptr<Program> parseSource(const std::string& source,
                                     const std::string& filename,
                                     std::vector<std::string>& errors) {
    g_parse_errors = &errors;
    myl_set_input(source.c_str(), filename.c_str());

    Program* result = nullptr;
    int rc = yyparse(result);

    myl_cleanup_input();
    g_parse_errors = nullptr;

    if (rc != 0 || result == nullptr) {
        delete result;
        return nullptr;
    }
    return std::unique_ptr<Program>(result);
}

} // namespace myl
