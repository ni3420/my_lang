#pragma once

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <llvm/IR/Value.h>

namespace myl {

struct Location {
    int line = 1;
    int column = 1;
    std::string filename;

    Location() = default;
    Location(int l, int c, const std::string& f) : line(l), column(c), filename(f) {}
};

enum class TypeKind {
    Void,
    Int,
    Float,
    Bool,
    String
};

struct Type {
    TypeKind kind = TypeKind::Void;

    Type() = default;
    explicit Type(TypeKind k) : kind(k) {}

    bool operator==(const Type& other) const { return kind == other.kind; }
    bool operator!=(const Type& other) const { return kind != other.kind; }

    static Type voidType() { return Type(TypeKind::Void); }
    static Type intType() { return Type(TypeKind::Int); }
    static Type floatType() { return Type(TypeKind::Float); }
    static Type boolType() { return Type(TypeKind::Bool); }
    static Type stringType() { return Type(TypeKind::String); }

    std::string toString() const;
};

class Expr;
class Stmt;
class Decl;

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
using DeclPtr = std::unique_ptr<Decl>;

class Expr {
public:
    Location loc;
    Type type;

    Expr() = default;
    explicit Expr(const Location& l) : loc(l) {}
    virtual ~Expr() = default;

    virtual llvm::Value* codegen(class CodeGenContext& ctx) = 0;
};

class Stmt {
public:
    Location loc;

    Stmt() = default;
    explicit Stmt(const Location& l) : loc(l) {}
    virtual ~Stmt() = default;

    virtual void codegen(class CodeGenContext& ctx) = 0;
};

class Decl {
public:
    Location loc;

    Decl() = default;
    explicit Decl(const Location& l) : loc(l) {}
    virtual ~Decl() = default;

    virtual void codegen(class CodeGenContext& ctx) = 0;
};

class IntegerLiteral : public Expr {
public:
    long long value;

    IntegerLiteral(const Location& l, long long v) : Expr(l), value(v) {
        type = Type::intType();
    }
    llvm::Value* codegen(CodeGenContext& ctx) override;
};

class FloatLiteral : public Expr {
public:
    double value;

    FloatLiteral(const Location& l, double v) : Expr(l), value(v) {
        type = Type::floatType();
    }
    llvm::Value* codegen(CodeGenContext& ctx) override;
};

class BoolLiteral : public Expr {
public:
    bool value;

    BoolLiteral(const Location& l, bool v) : Expr(l), value(v) {
        type = Type::boolType();
    }
    llvm::Value* codegen(CodeGenContext& ctx) override;
};

class StringLiteral : public Expr {
public:
    std::string value;

    StringLiteral(const Location& l, const std::string& v) : Expr(l), value(v) {
        type = Type::stringType();
    }
    llvm::Value* codegen(CodeGenContext& ctx) override;
};

class Identifier : public Expr {
public:
    std::string name;

    Identifier(const Location& l, const std::string& n) : Expr(l), name(n) {}
    llvm::Value* codegen(CodeGenContext& ctx) override;
};

class BinaryExpr : public Expr {
public:
    enum class Op {
        Add, Sub, Mul, Div, Mod,
        Eq, Ne, Lt, Le, Gt, Ge,
        And, Or
    };

    Op op;
    ExprPtr left;
    ExprPtr right;

    BinaryExpr(const Location& l, Op o, ExprPtr&& lhs, ExprPtr&& rhs)
        : Expr(l), op(o), left(std::move(lhs)), right(std::move(rhs)) {}
    llvm::Value* codegen(CodeGenContext& ctx) override;
};

class UnaryExpr : public Expr {
public:
    enum class Op { Neg, Not };

    Op op;
    ExprPtr operand;

    UnaryExpr(const Location& l, Op o, ExprPtr&& e)
        : Expr(l), op(o), operand(std::move(e)) {}
    llvm::Value* codegen(CodeGenContext& ctx) override;
};

class CallExpr : public Expr {
public:
    std::string callee;
    std::vector<ExprPtr> args;

    CallExpr(const Location& l, const std::string& c, std::vector<ExprPtr>&& a)
        : Expr(l), callee(c), args(std::move(a)) {}
    llvm::Value* codegen(CodeGenContext& ctx) override;
};

// VarDecl is a Decl, but we need it in statement context too
// Use a wrapper approach: create a VarDeclStmt that holds a VarDecl
class VarDecl : public Decl {
public:
    std::string name;
    Type declaredType;
    ExprPtr initializer;
    bool mutable_ = false;

    VarDecl(const Location& l, const std::string& n, Type t, ExprPtr&& init, bool mut)
        : Decl(l), name(n), declaredType(t), initializer(std::move(init)), mutable_(mut) {}
    void codegen(CodeGenContext& ctx) override;
};

// Wrapper for VarDecl in statement position
class VarDeclStmt : public Stmt {
public:
    VarDecl* decl = nullptr;

    VarDeclStmt(const Location& l, VarDecl* d) : Stmt(l), decl(d) {}
    void codegen(CodeGenContext& ctx) override;
};

class FunctionDecl : public Decl {
public:
    std::string name;
    struct Param {
        std::string name;
        Type type;
    };
    std::vector<Param> params;
    Type returnType;
    std::vector<StmtPtr> body;

    FunctionDecl(const Location& l, const std::string& n, std::vector<Param>&& p, Type rt, std::vector<StmtPtr>&& b)
        : Decl(l), name(n), params(std::move(p)), returnType(rt), body(std::move(b)) {}
    void codegen(CodeGenContext& ctx) override;
};

class ReturnStmt : public Stmt {
public:
    ExprPtr value;

    ReturnStmt(const Location& l, ExprPtr&& v) : Stmt(l), value(std::move(v)) {}
    void codegen(CodeGenContext& ctx) override;
};

class ExprStmt : public Stmt {
public:
    ExprPtr expr;

    ExprStmt(const Location& l, ExprPtr&& e) : Stmt(l), expr(std::move(e)) {}
    void codegen(CodeGenContext& ctx) override;
};

class AssignStmt : public Stmt {
public:
    std::string name;
    ExprPtr value;

    AssignStmt(const Location& l, const std::string& n, ExprPtr&& v)
        : Stmt(l), name(n), value(std::move(v)) {}
    void codegen(CodeGenContext& ctx) override;
};

class IfStmt : public Stmt {
public:
    ExprPtr condition;
    std::vector<StmtPtr> thenBranch;
    std::vector<StmtPtr> elseBranch;

    IfStmt(const Location& l, ExprPtr&& c, std::vector<StmtPtr>&& t, std::vector<StmtPtr>&& e)
        : Stmt(l), condition(std::move(c)), thenBranch(std::move(t)), elseBranch(std::move(e)) {}
    void codegen(CodeGenContext& ctx) override;
};

class WhileStmt : public Stmt {
public:
    ExprPtr condition;
    std::vector<StmtPtr> body;

    WhileStmt(const Location& l, ExprPtr&& c, std::vector<StmtPtr>&& b)
        : Stmt(l), condition(std::move(c)), body(std::move(b)) {}
    void codegen(CodeGenContext& ctx) override;
};

class BlockStmt : public Stmt {
public:
    std::vector<StmtPtr> statements;

    BlockStmt(const Location& l, std::vector<StmtPtr>&& s) : Stmt(l), statements(std::move(s)) {}
    void codegen(CodeGenContext& ctx) override;
};

// A statement appearing in declaration position (script mode). The parser
// collects these into an implicit `main` function.
class StmtDecl : public Decl {
public:
    StmtPtr stmt;

    StmtDecl(const Location& l, StmtPtr&& s) : Decl(l), stmt(std::move(s)) {}
    void codegen(CodeGenContext& ctx) override;
};

class Program : public Decl {
public:
    std::vector<DeclPtr> declarations;

    Program() = default;
    void codegen(CodeGenContext& ctx) override;
};

} // namespace myl