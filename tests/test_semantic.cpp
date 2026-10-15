#include "myl/frontend.hpp"
#include "myl/semantic.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

static std::vector<std::string> analyze(const std::string& src) {
    std::vector<std::string> errors;
    auto ast = myl::parseSource(src, "test", errors);
    assert(ast != nullptr && "parse failed unexpectedly");
    myl::SemanticAnalyzer analyzer;
    return analyzer.analyze(ast.get());
}

void test_semantic_valid_program() {
    auto errors = analyze(R"(
        fn add(a: int, b: int) -> int { return a + b; }
        fn main() -> int {
            let x = 10;
            let y = 20;
            let z = add(x, y);
            return z;
        }
    )");
    assert(errors.empty());
    std::cout << "test_semantic_valid_program: PASS\n";
}

void test_semantic_undefined_var() {
    auto errors = analyze("fn main() -> int { let x = y; return 0; }");
    assert(!errors.empty());
    std::cout << "test_semantic_undefined_var: PASS\n";
}

void test_semantic_type_mismatch() {
    auto errors = analyze("fn main() -> int { let x: int = \"hello\"; return 0; }");
    assert(!errors.empty());
    std::cout << "test_semantic_type_mismatch: PASS\n";
}

void test_semantic_redefinition() {
    auto errors = analyze("fn main() -> int { let x = 10; let x = 20; return 0; }");
    assert(!errors.empty());
    std::cout << "test_semantic_redefinition: PASS\n";
}

void test_semantic_if_condition_type() {
    auto errors = analyze("fn main() -> int { if (10) { } return 0; }");
    assert(!errors.empty());
    std::cout << "test_semantic_if_condition_type: PASS\n";
}

void test_semantic_while_condition_type() {
    auto errors = analyze("fn main() -> int { while (10) { } return 0; }");
    assert(!errors.empty());
    std::cout << "test_semantic_while_condition_type: PASS\n";
}

void test_semantic_return_type_mismatch() {
    auto errors = analyze("fn main() -> int { return \"hello\"; }");
    assert(!errors.empty());
    std::cout << "test_semantic_return_type_mismatch: PASS\n";
}

void test_semantic_missing_return() {
    auto errors = analyze("fn main() -> int { let x = 10; }");
    assert(!errors.empty());
    std::cout << "test_semantic_missing_return: PASS\n";
}

void test_semantic_void_function() {
    auto errors = analyze("fn main() -> void { return; }");
    assert(errors.empty());
    std::cout << "test_semantic_void_function: PASS\n";
}

void test_semantic_assign_immutable() {
    auto errors = analyze("fn main() -> int { let x = 10; x = 20; return x; }");
    assert(!errors.empty());
    std::cout << "test_semantic_assign_immutable: PASS\n";
}

void test_semantic_argument_count() {
    auto errors = analyze("fn add(a: int, b: int) -> int { return a + b; } "
                          "fn main() -> int { return add(1); }");
    assert(!errors.empty());
    std::cout << "test_semantic_argument_count: PASS\n";
}

void test_semantic_hindi_script_valid() {
    auto errors = analyze(R"(
        rakho x = 10;
        rakho y = 20;
        bolo "Namaste Duniya!";
        bolo jodo(x, guna(y, 3));
        agar (x > 5) { bolo "sahi"; } warna { bolo "galat"; }
        repeat (x > 0) { x = ghatao(x, 1); }
    )");
    assert(errors.empty());
    std::cout << "test_semantic_hindi_script_valid: PASS\n";
}

void test_semantic_hindi_function_valid() {
    auto errors = analyze(R"(
        kaam badhao(a: ank, b: ank) -> ank { bhejo jodo(a, b); }
        kaam main() -> ank {
            rakho r = badhao(1, 2);
            agar (r == 3) { bolo sahi; }
            bhejo 0;
        }
    )");
    assert(errors.empty());
    std::cout << "test_semantic_hindi_function_valid: PASS\n";
}

void test_semantic_builtin_type_error() {
    auto errors = analyze("fn main() -> int { print(jodo(10, \"hello\")); return 0; }");
    assert(!errors.empty());
    std::cout << "test_semantic_builtin_type_error: PASS\n";
}

void test_semantic_builtin_arity_error() {
    auto errors = analyze("fn main() -> int { print(guna(10)); return 0; }");
    assert(!errors.empty());
    std::cout << "test_semantic_builtin_arity_error: PASS\n";
}

int main() {
    test_semantic_valid_program();
    test_semantic_undefined_var();
    test_semantic_type_mismatch();
    test_semantic_redefinition();
    test_semantic_if_condition_type();
    test_semantic_while_condition_type();
    test_semantic_return_type_mismatch();
    test_semantic_missing_return();
    test_semantic_void_function();
    test_semantic_assign_immutable();
    test_semantic_argument_count();
    test_semantic_hindi_script_valid();
    test_semantic_hindi_function_valid();
    test_semantic_builtin_type_error();
    test_semantic_builtin_arity_error();
    std::cout << "All semantic tests passed!\n";
    return 0;
}
