#include "myl/frontend.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

static std::unique_ptr<myl::Program> parse(const std::string& src,
                                           std::vector<std::string>& errors) {
    return myl::parseSource(src, "test", errors);
}

void test_parse_simple_var() {
    std::vector<std::string> errors;
    auto ast = parse("let x = 10;", errors);
    assert(ast != nullptr);
    assert(errors.empty());
    assert(ast->declarations.size() == 1);
    std::cout << "test_parse_simple_var: PASS\n";
}

void test_parse_var_with_type() {
    std::vector<std::string> errors;
    auto ast = parse("let x: int = 10;", errors);
    assert(ast != nullptr);
    assert(errors.empty());
    std::cout << "test_parse_var_with_type: PASS\n";
}

void test_parse_mutable_var() {
    std::vector<std::string> errors;
    auto ast = parse("let mut x = 10;", errors);
    assert(ast != nullptr);
    assert(errors.empty());
    std::cout << "test_parse_mutable_var: PASS\n";
}

void test_parse_function() {
    std::vector<std::string> errors;
    auto ast = parse("fn add(a: int, b: int) -> int { return a + b; }", errors);
    assert(ast != nullptr);
    assert(errors.empty());
    assert(ast->declarations.size() == 1);
    std::cout << "test_parse_function: PASS\n";
}

void test_parse_if_stmt() {
    std::vector<std::string> errors;
    auto ast = parse("fn f(x: int) -> int { if (x > 10) { return 1; } else { return 0; } }", errors);
    assert(ast != nullptr);
    assert(errors.empty());
    std::cout << "test_parse_if_stmt: PASS\n";
}

void test_parse_while_stmt() {
    std::vector<std::string> errors;
    auto ast = parse("fn f() -> int { let mut x = 3; while (x > 0) { x = x - 1; } return x; }", errors);
    assert(ast != nullptr);
    assert(errors.empty());
    std::cout << "test_parse_while_stmt: PASS\n";
}

void test_parse_precedence() {
    std::vector<std::string> errors;
    auto ast = parse("let x = 10 + 20 * 3;", errors);
    assert(ast != nullptr);
    assert(errors.empty());
    std::cout << "test_parse_precedence: PASS\n";
}

void test_parse_call_expr() {
    std::vector<std::string> errors;
    auto ast = parse("fn add(a: int, b: int) -> int { return a + b; } fn main() -> int { return add(1, 2); }", errors);
    assert(ast != nullptr);
    assert(errors.empty());
    std::cout << "test_parse_call_expr: PASS\n";
}

void test_parse_invalid_syntax() {
    std::vector<std::string> errors;
    auto ast = parse("let x = ;", errors);
    assert(ast == nullptr);
    assert(!errors.empty());
    std::cout << "test_parse_invalid_syntax: PASS\n";
}

void test_parse_hindi_script() {
    std::vector<std::string> errors;
    auto ast = parse("rakho x = 10;\nbolo jodo(x, guna(2, 3));", errors);
    assert(ast != nullptr);
    assert(errors.empty());
    // Top-level statements are wrapped into a single implicit `main`.
    assert(ast->declarations.size() == 1);
    assert(dynamic_cast<myl::FunctionDecl*>(ast->declarations[0].get()) != nullptr);
    std::cout << "test_parse_hindi_script: PASS\n";
}

void test_parse_hindi_function() {
    std::vector<std::string> errors;
    auto ast = parse("kaam badhao(a: ank, b: ank) -> ank { bhejo jodo(a, b); }", errors);
    assert(ast != nullptr);
    assert(errors.empty());
    assert(ast->declarations.size() == 1);
    std::cout << "test_parse_hindi_function: PASS\n";
}

void test_parse_hindi_control_flow() {
    std::vector<std::string> errors;
    auto ast = parse("kaam main() -> ank { rakho x = 3; agar (x > 1) { bolo sahi; } "
                     "warna { bolo galat; } repeat (x > 0) { x = ghatao(x, 1); } bhejo 0; }",
                     errors);
    assert(ast != nullptr);
    assert(errors.empty());
    std::cout << "test_parse_hindi_control_flow: PASS\n";
}

int main() {
    test_parse_simple_var();
    test_parse_var_with_type();
    test_parse_mutable_var();
    test_parse_function();
    test_parse_if_stmt();
    test_parse_while_stmt();
    test_parse_precedence();
    test_parse_call_expr();
    test_parse_invalid_syntax();
    test_parse_hindi_script();
    test_parse_hindi_function();
    test_parse_hindi_control_flow();
    std::cout << "All parser tests passed!\n";
    return 0;
}
