#include "myl/driver.hpp"
#include "myl/frontend.hpp"
#include "myl/semantic.hpp"
#include "myl/codegen.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

#ifndef _WIN32
#include <sys/wait.h>
#endif

namespace myl {

Driver::Driver(const CompileOptions& opts) : options_(opts) {}

int Driver::run() {
    if (options_.inputFile.empty()) {
        printHelp();
        return 1;
    }

    if (!readSource()) return 1;
    if (!lexAndParse()) return 1;
    if (!semanticAnalysis()) return 1;
    if (!codegen()) return 1;
    if (!writeOutput()) return 1;

    return 0;
}

bool Driver::readSource() {
    std::ifstream file(options_.inputFile);
    if (!file) {
        std::cerr << "error: cannot open file '" << options_.inputFile << "'\n";
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    source_ = buffer.str();
    return true;
}

bool Driver::lexAndParse() {
    errors_.clear();
    ast_ = parseSource(source_, options_.inputFile, errors_);
    if (!errors_.empty() || !ast_) {
        if (errors_.empty()) {
            errors_.push_back("error: parsing failed");
        }
        printErrors();
        return false;
    }
    return true;
}

bool Driver::semanticAnalysis() {
    SemanticAnalyzer analyzer;
    errors_ = analyzer.analyze(ast_.get());
    if (!errors_.empty()) {
        printErrors();
        return false;
    }
    return true;
}

bool Driver::codegen() {
    CodeGenContext ctx;
    ast_->codegen(ctx);

    if (!ctx.verifyModule()) {
        std::cerr << "error: LLVM IR verification failed\n";
        return false;
    }

    switch (options_.mode) {
        case CompileMode::Check:
            return true;
        case CompileMode::EmitIR:
            if (options_.outputFile.empty()) options_.outputFile = "out.ll";
            return ctx.emitIR(options_.outputFile);
        case CompileMode::Build:
            if (options_.outputFile.empty()) options_.outputFile = "out";
            return ctx.emitExecutable(options_.outputFile);
        case CompileMode::Run:
            if (!ctx.emitExecutable("/tmp/myl_run_out")) return false;
            return true;
    }
    return false;
}

bool Driver::writeOutput() {
    if (options_.mode == CompileMode::Run) {
        int result = std::system("/tmp/myl_run_out");
        std::remove("/tmp/myl_run_out");
        return WIFEXITED(result) && WEXITSTATUS(result) == 0;
    }
    return true;
}

void Driver::printErrors() const {
    for (const auto& err : errors_) {
        std::cerr << err << "\n";
    }
}

void Driver::printVersion() const {
    std::cout << "myl 0.1.0\n";
}

void Driver::printHelp() const {
    std::cout << "Usage: myl [options] <file.myl>\n\n";
    std::cout << "Options:\n";
    std::cout << "  --help          Show this help message\n";
    std::cout << "  --version       Show version information\n";
    std::cout << "  check <file>    Parse and type-check only\n";
    std::cout << "  emit-ir <file>  Emit LLVM IR to file\n";
    std::cout << "  build <file>    Build native executable\n";
    std::cout << "  run <file>      Compile and run\n";
}

} // namespace myl