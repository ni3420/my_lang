#include "myl/semantic.hpp"
#include "myl/ast.hpp"
#include <sstream>

namespace myl {

std::vector<std::string> SemanticAnalyzer::analyze(Program* program) {
    declareBuiltins();
    visitProgram(program);
    return errors_;
}

void SemanticAnalyzer::error(const Location& loc, const std::string& msg) {
    std::ostringstream oss;
    oss << loc.filename << ":" << loc.line << ":" << loc.column << ": " << msg;
    errors_.push_back(oss.str());
}

void SemanticAnalyzer::error(const std::string& msg) {
    errors_.push_back(msg);
}

void SemanticAnalyzer::declareBuiltins() {
    currentScope_->define("print", Symbol{
        Symbol::Kind::Function,
        Type::voidType(),
        false, true, nullptr
    });
    // Arithmetic built-ins (Hindi aliases). Reserved so user code cannot
    // redefine them; their types are computed in visitCallExpr.
    for (const char* name : { "jodo", "ghatao", "guna", "bhaag" }) {
        currentScope_->define(name, Symbol{
            Symbol::Kind::Function,
            Type::intType(),
            false, true, nullptr
        });
    }
}

void SemanticAnalyzer::visitProgram(Program* prog) {
    for (auto& decl : prog->declarations) {
        if (auto* fn = dynamic_cast<FunctionDecl*>(decl.get())) {
            visitFunctionDecl(fn);
        } else if (auto* var = dynamic_cast<VarDecl*>(decl.get())) {
            visitVarDecl(var);
        }
    }
}

void SemanticAnalyzer::visitStmt(Stmt* stmt) {
    if (auto* r = dynamic_cast<ReturnStmt*>(stmt)) visitReturnStmt(r);
    else if (auto* e = dynamic_cast<ExprStmt*>(stmt)) visitExprStmt(e);
    else if (auto* i = dynamic_cast<IfStmt*>(stmt)) visitIfStmt(i);
    else if (auto* w = dynamic_cast<WhileStmt*>(stmt)) visitWhileStmt(w);
    else if (auto* b = dynamic_cast<BlockStmt*>(stmt)) visitBlockStmt(b);
    else if (auto* v = dynamic_cast<VarDeclStmt*>(stmt)) visitVarDeclStmt(v);
    else if (auto* a = dynamic_cast<AssignStmt*>(stmt)) visitAssignStmt(a);
}

void SemanticAnalyzer::visitVarDeclStmt(VarDeclStmt* stmt) {
    if (stmt->decl) visitVarDecl(stmt->decl);
}

void SemanticAnalyzer::visitAssignStmt(AssignStmt* stmt) {
    auto symOpt = currentScope_->lookup(stmt->name);
    if (!symOpt) {
        error(stmt->loc, "assignment to undefined variable '" + stmt->name + "'");
        return;
    }
    auto sym = *symOpt;
    if (sym->kind == Symbol::Kind::Function) {
        error(stmt->loc, "'" + stmt->name + "' is not a variable");
        return;
    }
    if (!sym->mutable_) {
        error(stmt->loc, "cannot assign to immutable variable '" + stmt->name +
                         "' (declare it with 'let mut')");
        return;
    }
    auto valType = visitExpr(stmt->value.get());
    checkTypeCompatibility(sym->type, valType, stmt->loc, "assignment");
}

void SemanticAnalyzer::visitVarDecl(VarDecl* decl) {
    Type initType = Type::voidType();
    if (decl->initializer) {
        initType = visitExpr(decl->initializer.get());
    }

    if (decl->declaredType != Type::voidType()) {
        if (initType != Type::voidType() && !checkTypeCompatibility(decl->declaredType, initType, decl->loc, "variable initialization")) {
            return;
        }
    } else {
        if (initType == Type::voidType()) {
            error(decl->loc, "cannot infer type of variable '" + decl->name + "' without initializer");
            return;
        }
        decl->declaredType = initType;
    }

    Symbol sym{ Symbol::Kind::Variable, decl->declaredType, decl->mutable_, true, nullptr };
    if (!currentScope_->lookupCurrent(decl->name)) {
        currentScope_->define(decl->name, std::move(sym));
    } else {
        error(decl->loc, "redefinition of variable '" + decl->name + "'");
    }
}

void SemanticAnalyzer::visitFunctionDecl(FunctionDecl* decl) {
    if (currentScope_->lookupCurrent(decl->name)) {
        error(decl->loc, "redefinition of function '" + decl->name + "'");
        return;
    }

    Symbol fnSym{ Symbol::Kind::Function, decl->returnType, false, true, nullptr, decl };
    currentScope_->define(decl->name, std::move(fnSym));

    enterScope();
    currentFunction_ = decl;

    for (auto& param : decl->params) {
        Symbol paramSym{ Symbol::Kind::Parameter, param.type, false, true, nullptr, nullptr };
        currentScope_->define(param.name, std::move(paramSym));
    }

    for (auto& stmt : decl->body) {
        visitStmt(stmt.get());
    }

    bool hasReturn = false;
    for (auto& stmt : decl->body) {
        if (dynamic_cast<ReturnStmt*>(stmt.get())) {
            hasReturn = true;
            break;
        }
    }
    if (decl->returnType != Type::voidType() && !hasReturn) {
        error(decl->loc, "function '" + decl->name + "' missing return statement");
    }

    currentFunction_ = nullptr;
    exitScope();
}

void SemanticAnalyzer::visitReturnStmt(ReturnStmt* stmt) {
    if (!currentFunction_) {
        error(stmt->loc, "return statement outside of function");
        return;
    }
    if (stmt->value) {
        auto valType = visitExpr(stmt->value.get());
        checkTypeCompatibility(currentFunction_->returnType, valType, stmt->loc, "return statement");
    } else {
        checkTypeCompatibility(currentFunction_->returnType, Type::voidType(), stmt->loc, "return statement");
    }
}

void SemanticAnalyzer::visitExprStmt(ExprStmt* stmt) {
    if (stmt->expr) visitExpr(stmt->expr.get());
}

void SemanticAnalyzer::visitIfStmt(IfStmt* stmt) {
    auto condType = visitExpr(stmt->condition.get());
    if (!isBooleanType(condType)) {
        error(stmt->condition->loc, "if condition must be boolean");
    }

    enterScope();
    for (auto& s : stmt->thenBranch) visitStmt(s.get());
    exitScope();

    enterScope();
    for (auto& s : stmt->elseBranch) visitStmt(s.get());
    exitScope();
}

void SemanticAnalyzer::visitWhileStmt(WhileStmt* stmt) {
    auto condType = visitExpr(stmt->condition.get());
    if (!isBooleanType(condType)) {
        error(stmt->condition->loc, "while condition must be boolean");
    }

    enterScope();
    for (auto& s : stmt->body) visitStmt(s.get());
    exitScope();
}

void SemanticAnalyzer::visitBlockStmt(BlockStmt* stmt) {
    enterScope();
    for (auto& s : stmt->statements) visitStmt(s.get());
    exitScope();
}

Type SemanticAnalyzer::visitExpr(Expr* expr) {
    if (auto* e = dynamic_cast<BinaryExpr*>(expr)) return visitBinaryExpr(e);
    if (auto* e = dynamic_cast<UnaryExpr*>(expr)) return visitUnaryExpr(e);
    if (auto* e = dynamic_cast<CallExpr*>(expr)) return visitCallExpr(e);
    if (auto* e = dynamic_cast<Identifier*>(expr)) return visitIdentifier(e);
    if (auto* e = dynamic_cast<IntegerLiteral*>(expr)) return visitIntegerLiteral(e);
    if (auto* e = dynamic_cast<FloatLiteral*>(expr)) return visitFloatLiteral(e);
    if (auto* e = dynamic_cast<BoolLiteral*>(expr)) return visitBoolLiteral(e);
    if (auto* e = dynamic_cast<StringLiteral*>(expr)) return visitStringLiteral(e);
    return Type::voidType();
}

Type SemanticAnalyzer::visitBinaryExpr(BinaryExpr* expr) {
    auto leftType = visitExpr(expr->left.get());
    auto rightType = visitExpr(expr->right.get());

    bool isArithmetic = expr->op == BinaryExpr::Op::Add || expr->op == BinaryExpr::Op::Sub ||
                        expr->op == BinaryExpr::Op::Mul || expr->op == BinaryExpr::Op::Div ||
                        expr->op == BinaryExpr::Op::Mod;
    bool isComparison = expr->op == BinaryExpr::Op::Eq || expr->op == BinaryExpr::Op::Ne ||
                        expr->op == BinaryExpr::Op::Lt || expr->op == BinaryExpr::Op::Le ||
                        expr->op == BinaryExpr::Op::Gt || expr->op == BinaryExpr::Op::Ge;
    bool isLogical = expr->op == BinaryExpr::Op::And || expr->op == BinaryExpr::Op::Or;

    if (isArithmetic) {
        if (!isNumericType(leftType) || !isNumericType(rightType)) {
            error(expr->loc, "arithmetic operands must be numeric");
            return Type::voidType();
        }
        expr->type = commonType(leftType, rightType);
        return expr->type;
    }

    if (isComparison) {
        if (!isNumericType(leftType) || !isNumericType(rightType)) {
            error(expr->loc, "comparison operands must be numeric");
            return Type::boolType();
        }
        expr->type = Type::boolType();
        return Type::boolType();
    }

    if (isLogical) {
        if (!isBooleanType(leftType) || !isBooleanType(rightType)) {
            error(expr->loc, "logical operands must be boolean");
            return Type::boolType();
        }
        expr->type = Type::boolType();
        return Type::boolType();
    }

    return Type::voidType();
}

Type SemanticAnalyzer::visitUnaryExpr(UnaryExpr* expr) {
    auto operandType = visitExpr(expr->operand.get());

    if (expr->op == UnaryExpr::Op::Neg) {
        if (!isNumericType(operandType)) {
            error(expr->loc, "unary minus requires numeric operand");
            return Type::voidType();
        }
        expr->type = operandType;
        return operandType;
    }

    if (expr->op == UnaryExpr::Op::Not) {
        if (!isBooleanType(operandType)) {
            error(expr->loc, "logical not requires boolean operand");
            return Type::boolType();
        }
        expr->type = Type::boolType();
        return Type::boolType();
    }

    return Type::voidType();
}

Type SemanticAnalyzer::visitCallExpr(CallExpr* expr) {
    if (expr->callee == "print") {
        for (auto& arg : expr->args) visitExpr(arg.get());
        expr->type = Type::voidType();
        return Type::voidType();
    }

    if (expr->callee == "jodo" || expr->callee == "ghatao" ||
        expr->callee == "guna" || expr->callee == "bhaag") {
        if (expr->args.size() != 2) {
            error(expr->loc, "builtin '" + expr->callee + "' expects 2 arguments, got " +
                             std::to_string(expr->args.size()));
            expr->type = Type::voidType();
            return Type::voidType();
        }
        auto leftType = visitExpr(expr->args[0].get());
        auto rightType = visitExpr(expr->args[1].get());
        if (!isNumericType(leftType) || !isNumericType(rightType)) {
            error(expr->loc, "builtin '" + expr->callee + "' operands must be numeric");
            expr->type = Type::voidType();
            return Type::voidType();
        }
        expr->type = commonType(leftType, rightType);
        return expr->type;
    }

    auto symOpt = currentScope_->lookup(expr->callee);
    if (!symOpt) {
        error(expr->loc, "undefined function '" + expr->callee + "'");
        return Type::voidType();
    }

    auto sym = *symOpt;
    if (sym->kind != Symbol::Kind::Function) {
        error(expr->loc, "'" + expr->callee + "' is not a function");
        return Type::voidType();
    }

    if (sym->functionDecl) {
        auto* fd = sym->functionDecl;
        if (expr->args.size() != fd->params.size()) {
            error(expr->loc, "function '" + expr->callee + "' expects " +
                             std::to_string(fd->params.size()) + " argument(s), got " +
                             std::to_string(expr->args.size()));
        } else {
            for (size_t i = 0; i < fd->params.size(); ++i) {
                auto argType = visitExpr(expr->args[i].get());
                checkTypeCompatibility(fd->params[i].type, argType, expr->loc,
                                       "argument " + std::to_string(i + 1) +
                                       " of '" + expr->callee + "'");
            }
        }
    } else {
        for (auto& arg : expr->args) visitExpr(arg.get());
    }

    expr->type = sym->type;
    return sym->type;
}

Type SemanticAnalyzer::visitIdentifier(Identifier* expr) {
    auto symOpt = currentScope_->lookup(expr->name);
    if (!symOpt) {
        error(expr->loc, "undefined variable '" + expr->name + "'");
        return Type::voidType();
    }

    auto sym = *symOpt;
    if (sym->kind == Symbol::Kind::Function) {
        error(expr->loc, "function '" + expr->name + "' used as value");
        return Type::voidType();
    }

    if (!sym->initialized) {
        error(expr->loc, "use of uninitialized variable '" + expr->name + "'");
    }

    expr->type = sym->type;
    return sym->type;
}

Type SemanticAnalyzer::visitIntegerLiteral(IntegerLiteral* expr) {
    expr->type = Type::intType();
    return Type::intType();
}

Type SemanticAnalyzer::visitFloatLiteral(FloatLiteral* expr) {
    expr->type = Type::floatType();
    return Type::floatType();
}

Type SemanticAnalyzer::visitBoolLiteral(BoolLiteral* expr) {
    expr->type = Type::boolType();
    return Type::boolType();
}

Type SemanticAnalyzer::visitStringLiteral(StringLiteral* expr) {
    expr->type = Type::stringType();
    return Type::stringType();
}

bool SemanticAnalyzer::checkTypeCompatibility(Type expected, Type actual, const Location& loc, const std::string& context) {
    if (expected == Type::voidType() || actual == Type::voidType()) return true;
    if (expected == actual) return true;
    if (isNumericType(expected) && isNumericType(actual)) return true;
    error(loc, "type mismatch in " + context + ": expected " + expected.toString() + ", got " + actual.toString());
    return false;
}

Type SemanticAnalyzer::commonType(Type a, Type b) {
    if (a == Type::floatType() || b == Type::floatType()) return Type::floatType();
    return Type::intType();
}

bool SemanticAnalyzer::isNumericType(Type t) {
    return t == Type::intType() || t == Type::floatType();
}

bool SemanticAnalyzer::isBooleanType(Type t) {
    return t == Type::boolType();
}

} // namespace myl