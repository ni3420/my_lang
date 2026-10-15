#pragma once

#include "myl/ast.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

namespace myl {

struct Symbol {
    enum class Kind { Variable, Function, Parameter } kind;
    Type type;
    bool mutable_ = false;
    bool initialized = false;
    llvm::Value* llvmValue = nullptr;
    FunctionDecl* functionDecl = nullptr;
};

class Scope {
public:
    explicit Scope(Scope* parent = nullptr) : parent_(parent) {}

    void define(const std::string& name, Symbol sym) {
        symbols_[name] = std::move(sym);
    }

    std::optional<Symbol*> lookup(const std::string& name) {
        auto it = symbols_.find(name);
        if (it != symbols_.end()) return &it->second;
        if (parent_) return parent_->lookup(name);
        return std::nullopt;
    }

    std::optional<Symbol*> lookupCurrent(const std::string& name) {
        auto it = symbols_.find(name);
        if (it != symbols_.end()) return &it->second;
        return std::nullopt;
    }

private:
    Scope* parent_;
    std::unordered_map<std::string, Symbol> symbols_;
};

class SemanticAnalyzer {
public:
    SemanticAnalyzer() { enterScope(); }
    ~SemanticAnalyzer() { exitScope(); }

    std::vector<std::string> analyze(Program* program);

private:
    std::vector<std::string> errors_;
    Scope* currentScope_ = nullptr;
    FunctionDecl* currentFunction_ = nullptr;
    std::vector<Scope*> scopeStack_;

    void enterScope() {
        scopeStack_.push_back(new Scope(currentScope_));
        currentScope_ = scopeStack_.back();
    }

    void exitScope() {
        if (!scopeStack_.empty()) {
            delete scopeStack_.back();
            scopeStack_.pop_back();
        }
        currentScope_ = scopeStack_.empty() ? nullptr : scopeStack_.back();
    }

    void error(const Location& loc, const std::string& msg);
    void error(const std::string& msg);

    void visitProgram(Program* prog);
    void visitStmt(Stmt* stmt);
    void visitVarDecl(VarDecl* decl);
    void visitVarDeclStmt(VarDeclStmt* stmt);
    void visitAssignStmt(AssignStmt* stmt);
    void visitFunctionDecl(FunctionDecl* decl);
    void visitReturnStmt(ReturnStmt* stmt);
    void visitExprStmt(ExprStmt* stmt);
    void visitIfStmt(IfStmt* stmt);
    void visitWhileStmt(WhileStmt* stmt);
    void visitBlockStmt(BlockStmt* stmt);
    Type visitExpr(Expr* expr);
    Type visitBinaryExpr(BinaryExpr* expr);
    Type visitUnaryExpr(UnaryExpr* expr);
    Type visitCallExpr(CallExpr* expr);
    Type visitIdentifier(Identifier* expr);
    Type visitIntegerLiteral(IntegerLiteral* expr);
    Type visitFloatLiteral(FloatLiteral* expr);
    Type visitBoolLiteral(BoolLiteral* expr);
    Type visitStringLiteral(StringLiteral* expr);

    bool checkTypeCompatibility(Type expected, Type actual, const Location& loc, const std::string& context);
    Type commonType(Type a, Type b);
    bool isNumericType(Type t);
    bool isBooleanType(Type t);
    void declareBuiltins();
};

} // namespace myl