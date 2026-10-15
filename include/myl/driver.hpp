#pragma once

#include "myl/ast.hpp"
#include "myl/frontend.hpp"
#include "myl/semantic.hpp"
#include "myl/codegen.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

namespace myl {

enum class CompileMode {
    Check,
    EmitIR,
    Build,
    Run
};

struct CompileOptions {
    CompileMode mode = CompileMode::Check;
    std::string inputFile;
    std::string outputFile;
    bool verbose = false;
};

class Driver {
public:
    explicit Driver(const CompileOptions& opts);
    int run();

    void printVersion() const;
    void printHelp() const;

private:
    CompileOptions options_;
    std::string source_;
    std::unique_ptr<Program> ast_;
    std::vector<std::string> errors_;

    bool readSource();
    bool lexAndParse();
    bool semanticAnalysis();
    bool codegen();
    bool writeOutput();

    void printErrors() const;
};

} // namespace myl