#include "myl/frontend.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

static std::vector<myl::TokenInfo> lex(const std::string& src) {
    std::vector<std::string> errors;
    auto toks = myl::tokenize(src, "test", errors);
    assert(errors.empty());
    return toks;
}

void test_tokenize_integer() {
    auto toks = lex("123");
    assert(toks.size() == 1);
    assert(toks[0].kind == "integer");
    assert(toks[0].text == "123");
    std::cout << "test_tokenize_integer: PASS\n";
}

void test_tokenize_float() {
    auto toks = lex("3.14");
    assert(toks.size() == 1);
    assert(toks[0].kind == "float_literal");
    assert(toks[0].text == "3.14");
    std::cout << "test_tokenize_float: PASS\n";
}

void test_tokenize_string() {
    auto toks = lex(R"("hello world")");
    assert(toks.size() == 1);
    assert(toks[0].kind == "string_literal");
    assert(toks[0].text == "hello world");
    std::cout << "test_tokenize_string: PASS\n";
}

void test_tokenize_identifier() {
    auto toks = lex("myVar");
    assert(toks.size() == 1);
    assert(toks[0].kind == "identifier");
    assert(toks[0].text == "myVar");
    std::cout << "test_tokenize_identifier: PASS\n";
}

void test_tokenize_keywords() {
    auto toks = lex("let mut fn return if else while print int float bool string void true false and or not");
    std::vector<std::string> expected = {
        "let", "mut", "fn", "return", "if", "else", "while", "print",
        "int", "float", "bool", "string", "void", "true", "false", "and", "or", "not"
    };
    assert(toks.size() == expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        assert(toks[i].kind == expected[i]);
    }
    std::cout << "test_tokenize_keywords: PASS\n";
}

void test_tokenize_hindi_keywords() {
    auto toks = lex("rakho bolo agar warna repeat kaam bhejo sahi galat ank");
    std::vector<std::string> expected = {
        "rakho", "bolo", "agar", "warna", "repeat", "kaam", "bhejo", "sahi", "galat", "ank"
    };
    assert(toks.size() == expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        assert(toks[i].kind == expected[i]);
    }
    std::cout << "test_tokenize_hindi_keywords: PASS\n";
}

void test_tokenize_operators() {
    auto toks = lex("+ - * / % == != < <= > >= = ; : , ( ) { } ->");
    std::vector<std::string> expected = {
        "+", "-", "*", "/", "%", "==", "!=", "<", "<=", ">", ">=",
        "=", ";", ":", ",", "(", ")", "{", "}", "->"
    };
    assert(toks.size() == expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        assert(toks[i].kind == expected[i]);
    }
    std::cout << "test_tokenize_operators: PASS\n";
}

void test_comments_skipped() {
    auto toks = lex("// a comment\n42 /* block */ 7");
    assert(toks.size() == 2);
    assert(toks[0].text == "42");
    assert(toks[1].text == "7");
    std::cout << "test_comments_skipped: PASS\n";
}

void test_locations() {
    auto toks = lex("let\n  x");
    assert(toks.size() == 2);
    assert(toks[0].line == 1);
    assert(toks[1].line == 2);
    std::cout << "test_locations: PASS\n";
}

int main() {
    test_tokenize_integer();
    test_tokenize_float();
    test_tokenize_string();
    test_tokenize_identifier();
    test_tokenize_keywords();
    test_tokenize_hindi_keywords();
    test_tokenize_operators();
    test_comments_skipped();
    test_locations();
    std::cout << "All lexer tests passed!\n";
    return 0;
}
