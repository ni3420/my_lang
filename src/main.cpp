#include "myl/driver.hpp"
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: myl [--help|--version|check|emit-ir|build|run] <file.myl> [-o output]\n";
        return 1;
    }

    myl::CompileOptions opts;

    std::vector<std::string> args(argv + 1, argv + argc);

    if (args[0] == "--help") {
        myl::Driver driver(opts);
        driver.printHelp();
        return 0;
    }
    if (args[0] == "--version") {
        myl::Driver driver(opts);
        driver.printVersion();
        return 0;
    }

    std::string command = args[0];
    size_t argIdx = 1;

    if (command == "check") {
        opts.mode = myl::CompileMode::Check;
    } else if (command == "emit-ir") {
        opts.mode = myl::CompileMode::EmitIR;
    } else if (command == "build") {
        opts.mode = myl::CompileMode::Build;
    } else if (command == "run") {
        opts.mode = myl::CompileMode::Run;
    } else {
        opts.mode = myl::CompileMode::Check;
        argIdx = 0;
    }

    if (argIdx >= args.size()) {
        std::cerr << "error: missing input file\n";
        return 1;
    }

    opts.inputFile = args[argIdx++];

    while (argIdx < args.size()) {
        if (args[argIdx] == "-o" && argIdx + 1 < args.size()) {
            opts.outputFile = args[argIdx + 1];
            argIdx += 2;
        } else if (args[argIdx] == "-v" || args[argIdx] == "--verbose") {
            opts.verbose = true;
            argIdx++;
        } else {
            std::cerr << "error: unknown option '" << args[argIdx] << "'\n";
            return 1;
        }
    }

    myl::Driver driver(opts);
    return driver.run();
}