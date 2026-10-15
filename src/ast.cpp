#include "myl/ast.hpp"
#include "myl/codegen.hpp"
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>

namespace myl {

std::string Type::toString() const {
    switch (kind) {
        case TypeKind::Void: return "void";
        case TypeKind::Int: return "int";
        case TypeKind::Float: return "float";
        case TypeKind::Bool: return "bool";
        case TypeKind::String: return "string";
    }
    return "unknown";
}

llvm::Value* IntegerLiteral::codegen(CodeGenContext& ctx) {
    return llvm::ConstantInt::get(ctx.context(), llvm::APInt(64, value, true));
}

llvm::Value* FloatLiteral::codegen(CodeGenContext& ctx) {
    return llvm::ConstantFP::get(ctx.context(), llvm::APFloat(value));
}

llvm::Value* BoolLiteral::codegen(CodeGenContext& ctx) {
    return llvm::ConstantInt::get(ctx.context(), llvm::APInt(1, value, false));
}

llvm::Value* StringLiteral::codegen(CodeGenContext& ctx) {
    auto& builder = ctx.builder();
    auto strConstant = llvm::ConstantDataArray::getString(ctx.context(), value, true);
    auto global = new llvm::GlobalVariable(
        ctx.module(), strConstant->getType(), true,
        llvm::GlobalValue::PrivateLinkage, strConstant, ".str");
    global->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
    global->setAlignment(llvm::Align(1));

    auto zero = llvm::ConstantInt::get(ctx.context(), llvm::APInt(64, 0));
    llvm::Value* indices[] = { zero, zero };
    return builder.CreateInBoundsGEP(global->getValueType(), global, indices, "str_ptr");
}

llvm::Value* Identifier::codegen(CodeGenContext& ctx) {
    auto val = ctx.getVariable(name);
    if (!val) return nullptr;
    return ctx.builder().CreateLoad(ctx.llvmType(type), val, name.c_str());
}

llvm::Value* BinaryExpr::codegen(CodeGenContext& ctx) {
    auto& builder = ctx.builder();
    auto lhs = left->codegen(ctx);
    auto rhs = right->codegen(ctx);
    if (!lhs || !rhs) return nullptr;

    bool isFloat = left->type == Type::floatType() || right->type == Type::floatType();

    switch (op) {
        case Op::Add:
            return isFloat ? builder.CreateFAdd(lhs, rhs, "addtmp")
                           : builder.CreateAdd(lhs, rhs, "addtmp");
        case Op::Sub:
            return isFloat ? builder.CreateFSub(lhs, rhs, "subtmp")
                           : builder.CreateSub(lhs, rhs, "subtmp");
        case Op::Mul:
            return isFloat ? builder.CreateFMul(lhs, rhs, "multmp")
                           : builder.CreateMul(lhs, rhs, "multmp");
        case Op::Div:
            return isFloat ? builder.CreateFDiv(lhs, rhs, "divtmp")
                           : builder.CreateSDiv(lhs, rhs, "divtmp");
        case Op::Mod:
            return isFloat ? builder.CreateFRem(lhs, rhs, "modtmp")
                           : builder.CreateSRem(lhs, rhs, "modtmp");
        case Op::Eq:
            return isFloat ? builder.CreateFCmpUEQ(lhs, rhs, "eqtmp")
                           : builder.CreateICmpEQ(lhs, rhs, "eqtmp");
        case Op::Ne:
            return isFloat ? builder.CreateFCmpUNE(lhs, rhs, "netmp")
                           : builder.CreateICmpNE(lhs, rhs, "netmp");
        case Op::Lt:
            return isFloat ? builder.CreateFCmpULT(lhs, rhs, "lttmp")
                           : builder.CreateICmpSLT(lhs, rhs, "lttmp");
        case Op::Le:
            return isFloat ? builder.CreateFCmpULE(lhs, rhs, "letmp")
                           : builder.CreateICmpSLE(lhs, rhs, "letmp");
        case Op::Gt:
            return isFloat ? builder.CreateFCmpUGT(lhs, rhs, "gttmp")
                           : builder.CreateICmpSGT(lhs, rhs, "gttmp");
        case Op::Ge:
            return isFloat ? builder.CreateFCmpUGE(lhs, rhs, "getmp")
                           : builder.CreateICmpSGE(lhs, rhs, "getmp");
        case Op::And:
            return builder.CreateAnd(lhs, rhs, "andtmp");
        case Op::Or:
            return builder.CreateOr(lhs, rhs, "ortmp");
    }
    return nullptr;
}

llvm::Value* UnaryExpr::codegen(CodeGenContext& ctx) {
    auto& builder = ctx.builder();
    auto val = operand->codegen(ctx);
    if (!val) return nullptr;

    switch (op) {
        case Op::Neg:
            if (operand->type == Type::floatType())
                return builder.CreateFNeg(val, "negtmp");
            return builder.CreateNeg(val, "negtmp");
        case Op::Not:
            return builder.CreateNot(val, "nottmp");
    }
    return nullptr;
}

llvm::Value* CallExpr::codegen(CodeGenContext& ctx) {
    auto& builder = ctx.builder();

    if (callee == "print") {
        for (auto& arg : args) {
            auto val = arg->codegen(ctx);
            if (!val) return nullptr;
            ctx.emitPrint(val, arg->type);
        }
        return nullptr;
    }

    // Arithmetic built-ins (Hindi aliases): jodo/ghatao/guna/bhaag.
    if (callee == "jodo" || callee == "ghatao" ||
        callee == "guna" || callee == "bhaag") {
        if (args.size() != 2) return nullptr;

        auto lhs = args[0]->codegen(ctx);
        auto rhs = args[1]->codegen(ctx);
        if (!lhs || !rhs) return nullptr;

        bool isFloat = args[0]->type == Type::floatType() ||
                       args[1]->type == Type::floatType();

        if (isFloat) {
            auto toDouble = [&](llvm::Value* v, const Type& t) -> llvm::Value* {
                if (t == Type::floatType()) return v;
                return builder.CreateSIToFP(v, llvm::Type::getDoubleTy(ctx.context()),
                                            "tofloat");
            };
            lhs = toDouble(lhs, args[0]->type);
            rhs = toDouble(rhs, args[1]->type);
            if (callee == "jodo")   return builder.CreateFAdd(lhs, rhs, "fadd");
            if (callee == "ghatao") return builder.CreateFSub(lhs, rhs, "fsub");
            if (callee == "guna")   return builder.CreateFMul(lhs, rhs, "fmul");
            return builder.CreateFDiv(lhs, rhs, "fdiv");
        }

        if (callee == "jodo")   return builder.CreateAdd(lhs, rhs, "add");
        if (callee == "ghatao") return builder.CreateSub(lhs, rhs, "sub");
        if (callee == "guna")   return builder.CreateMul(lhs, rhs, "mul");
        return builder.CreateSDiv(lhs, rhs, "div");
    }

    auto fn = ctx.getFunction(callee);
    if (!fn) return nullptr;

    std::vector<llvm::Value*> args;
    args.reserve(this->args.size());
    for (auto& arg : this->args) {
        auto val = arg->codegen(ctx);
        if (!val) return nullptr;
        args.push_back(val);
    }
    return builder.CreateCall(fn, args, "calltmp");
}

void VarDecl::codegen(CodeGenContext& ctx) {
    auto& builder = ctx.builder();
    llvm::Value* initVal = nullptr;
    if (initializer) {
        initVal = initializer->codegen(ctx);
        if (!initVal) return;
    }

    auto alloca = builder.CreateAlloca(ctx.llvmType(declaredType), nullptr, name.c_str());
    if (initVal) {
        builder.CreateStore(initVal, alloca);
    }
    ctx.setVariable(name, alloca);
}

void FunctionDecl::codegen(CodeGenContext& ctx) {
    auto& builder = ctx.builder();

    std::vector<llvm::Type*> paramTypes;
    paramTypes.reserve(params.size());
    for (auto& p : params) {
        paramTypes.push_back(ctx.llvmType(p.type));
    }

    auto retType = ctx.llvmType(returnType);
    auto fnType = llvm::FunctionType::get(retType, paramTypes, false);
    auto fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, name, ctx.module());
    ctx.setFunction(name, fn);

    auto entry = llvm::BasicBlock::Create(ctx.context(), "entry", fn);
    builder.SetInsertPoint(entry);

    size_t i = 0;
    for (auto& arg : fn->args()) {
        if (i < params.size()) {
            auto alloca = builder.CreateAlloca(paramTypes[i], nullptr, params[i].name.c_str());
            builder.CreateStore(&arg, alloca);
            ctx.setVariable(params[i].name, alloca);
        }
        i++;
    }

    for (auto& stmt : body) {
        stmt->codegen(ctx);
    }

    if (!builder.GetInsertBlock()->getTerminator()) {
        if (returnType == Type::voidType()) {
            builder.CreateRetVoid();
        } else {
            builder.CreateUnreachable();
        }
    }

    builder.ClearInsertionPoint();
}

void ReturnStmt::codegen(CodeGenContext& ctx) {
    auto& builder = ctx.builder();
    if (value) {
        auto val = value->codegen(ctx);
        if (val) builder.CreateRet(val);
    } else {
        builder.CreateRetVoid();
    }
}

void VarDeclStmt::codegen(CodeGenContext& ctx) {
    if (decl) decl->codegen(ctx);
}

void ExprStmt::codegen(CodeGenContext& ctx) {
    if (expr) expr->codegen(ctx);
}

void AssignStmt::codegen(CodeGenContext& ctx) {
    auto val = value->codegen(ctx);
    if (!val) return;
    auto alloca = ctx.getVariable(name);
    if (!alloca) return;
    ctx.builder().CreateStore(val, alloca);
}

void IfStmt::codegen(CodeGenContext& ctx) {
    auto& builder = ctx.builder();
    auto fn = builder.GetInsertBlock()->getParent();

    auto condVal = condition->codegen(ctx);
    if (!condVal) return;

    auto thenBB = llvm::BasicBlock::Create(ctx.context(), "then", fn);
    auto elseBB = elseBranch.empty() ? nullptr : llvm::BasicBlock::Create(ctx.context(), "else", fn);
    auto mergeBB = llvm::BasicBlock::Create(ctx.context(), "ifcont", fn);

    if (elseBB) {
        builder.CreateCondBr(condVal, thenBB, elseBB);
    } else {
        builder.CreateCondBr(condVal, thenBB, mergeBB);
    }

    builder.SetInsertPoint(thenBB);
    for (auto& stmt : thenBranch) stmt->codegen(ctx);
    if (!builder.GetInsertBlock()->getTerminator())
        builder.CreateBr(mergeBB);

    if (elseBB) {
        builder.SetInsertPoint(elseBB);
        for (auto& stmt : elseBranch) stmt->codegen(ctx);
        if (!builder.GetInsertBlock()->getTerminator())
            builder.CreateBr(mergeBB);
    }

    builder.SetInsertPoint(mergeBB);
}

void WhileStmt::codegen(CodeGenContext& ctx) {
    auto& builder = ctx.builder();
    auto fn = builder.GetInsertBlock()->getParent();

    auto condBB = llvm::BasicBlock::Create(ctx.context(), "while.cond", fn);
    auto bodyBB = llvm::BasicBlock::Create(ctx.context(), "while.body", fn);
    auto afterBB = llvm::BasicBlock::Create(ctx.context(), "while.after", fn);

    builder.CreateBr(condBB);

    builder.SetInsertPoint(condBB);
    auto condVal = condition->codegen(ctx);
    if (!condVal) return;
    builder.CreateCondBr(condVal, bodyBB, afterBB);

    builder.SetInsertPoint(bodyBB);
    for (auto& stmt : body) stmt->codegen(ctx);
    if (!builder.GetInsertBlock()->getTerminator())
        builder.CreateBr(condBB);

    builder.SetInsertPoint(afterBB);
}

void BlockStmt::codegen(CodeGenContext& ctx) {
    for (auto& stmt : statements) stmt->codegen(ctx);
}

void StmtDecl::codegen(CodeGenContext& ctx) {
    if (stmt) stmt->codegen(ctx);
}

void Program::codegen(CodeGenContext& ctx) {
    for (auto& decl : declarations) decl->codegen(ctx);
}

} // namespace myl