#include "myl/frontend.hpp"
#include "myl/semantic.hpp"
#include "myl/codegen.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

static std::unique_ptr<myl::Program> build(const std::string& src) {
    std::vector<std::string> errors;
    auto ast = myl::parseSource(src, "test", errors);
    assert(ast != nullptr);
    myl::SemanticAnalyzer analyzer;
    auto semErrors = analyzer.analyze(ast.get());
    assert(semErrors.empty());
    return ast;
}

void test_codegen_simple() {
    auto ast = build(R"(
        fn main() -> int {
            let x = 10;
            let y = 20;
            return x + y;
        }
    )");
    myl::CodeGenContext ctx;
    ast->codegen(ctx);
    assert(ctx.verifyModule());
    std::cout << "test_codegen_simple: PASS\n";
}

void test_codegen_function_call() {
    auto ast = build(R"(
        fn add(a: int, b: int) -> int { return a + b; }
        fn main() -> int { return add(10, 20); }
    )");
    myl::CodeGenContext ctx;
    ast->codegen(ctx);
    assert(ctx.verifyModule());
    std::cout << "test_codegen_function_call: PASS\n";
}

void test_codegen_if_stmt() {
    auto ast = build(R"(
        fn main() -> int {
            let x = 10;
            if (x > 5) { return 1; } else { return 0; }
        }
    )");
    myl::CodeGenContext ctx;
    ast->codegen(ctx);
    assert(ctx.verifyModule());
    std::cout << "test_codegen_if_stmt: PASS\n";
}

void test_codegen_while_stmt() {
    auto ast = build(R"(
        fn main() -> int {
            let mut x = 10;
            while (x > 0) { x = x - 1; }
            return x;
        }
    )");
    myl::CodeGenContext ctx;
    ast->codegen(ctx);
    assert(ctx.verifyModule());
    std::cout << "test_codegen_while_stmt: PASS\n";
}

void test_codegen_print() {
    auto ast = build(R"(
        fn main() -> int {
            print(42);
            print("hi");
            return 0;
        }
    )");
    myl::CodeGenContext ctx;
    ast->codegen(ctx);
    assert(ctx.verifyModule());
    std::cout << "test_codegen_print: PASS\n";
}

void test_codegen_emit_ir() {
    auto ast = build("fn main() -> int { return 42; }");
    myl::CodeGenContext ctx;
    ast->codegen(ctx);
    assert(ctx.verifyModule());
    assert(ctx.emitIR("test_out.ll"));
    std::cout << "test_codegen_emit_ir: PASS\n";
}

int main() {
    test_codegen_simple();
    test_codegen_function_call();
    test_codegen_if_stmt();
    test_codegen_while_stmt();
    test_codegen_print();
    test_codegen_emit_ir();
    std::cout << "All codegen tests passed!\n";
    return 0;
}
