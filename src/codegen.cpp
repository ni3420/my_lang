#include "myl/codegen.hpp"
#include "myl/ast.hpp"
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Host.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/IR/Constants.h>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace myl {

CodeGenContext::CodeGenContext() {
    initializeModule();
    initializePassManager();
}

CodeGenContext::~CodeGenContext() = default;

void CodeGenContext::initializeModule() {
    module_ = std::make_unique<llvm::Module>("myl_module", ctx_);
    module_->setDataLayout("");
    builder_ = std::make_unique<llvm::IRBuilder<>>(ctx_);

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
}

void CodeGenContext::initializePassManager() {
    fpm_ = std::make_unique<llvm::legacy::FunctionPassManager>(module_.get());
    fpm_->add(llvm::createInstructionCombiningPass());
    fpm_->add(llvm::createReassociatePass());
    fpm_->add(llvm::createGVNPass());
    fpm_->add(llvm::createCFGSimplificationPass());
    fpm_->doInitialization();
}

llvm::Type* CodeGenContext::llvmType(Type type) {
    switch (type.kind) {
        case TypeKind::Void: return llvm::Type::getVoidTy(ctx_);
        case TypeKind::Int: return llvm::Type::getInt64Ty(ctx_);
        case TypeKind::Float: return llvm::Type::getDoubleTy(ctx_);
        case TypeKind::Bool: return llvm::Type::getInt1Ty(ctx_);
        case TypeKind::String: return llvm::Type::getInt8PtrTy(ctx_);
    }
    return llvm::Type::getVoidTy(ctx_);
}

llvm::Function* CodeGenContext::getFunction(const std::string& name) {
    auto it = functions_.find(name);
    if (it != functions_.end()) return it->second;
    return module_->getFunction(name);
}

void CodeGenContext::setFunction(const std::string& name, llvm::Function* fn) {
    functions_[name] = fn;
}

llvm::Value* CodeGenContext::getVariable(const std::string& name) {
    auto it = variables_.find(name);
    if (it != variables_.end()) return it->second;
    return nullptr;
}

void CodeGenContext::setVariable(const std::string& name, llvm::Value* val) {
    variables_[name] = val;
}

bool CodeGenContext::verifyModule() {
    std::string errorStr;
    llvm::raw_string_ostream errorStream(errorStr);
    if (llvm::verifyModule(*module_, &errorStream)) {
        return false;
    }
    return true;
}

void CodeGenContext::optimizeModule() {
    for (auto& fn : *module_) {
        if (!fn.isDeclaration()) {
            fpm_->run(fn);
        }
    }
}

llvm::TargetMachine* CodeGenContext::createTargetMachine() {
    std::string targetTriple = llvm::sys::getDefaultTargetTriple();
    module_->setTargetTriple(targetTriple);

    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    if (!target) {
        return nullptr;
    }

    auto cpu = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    auto rm = llvm::Optional<llvm::Reloc::Model>(llvm::Reloc::Static);
    return target->createTargetMachine(targetTriple, cpu, features, opt, rm);
}

void CodeGenContext::emitPrint(llvm::Value* value, Type type) {
    auto& b = *builder_;

    llvm::Function* printfFn = module_->getFunction("printf");
    if (!printfFn) {
        auto printfType = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(ctx_),
            { llvm::Type::getInt8PtrTy(ctx_) }, true);
        printfFn = llvm::Function::Create(printfType,
                                          llvm::Function::ExternalLinkage,
                                          "printf", module_.get());
    }

    const char* fmt = "%lld\n";
    llvm::Value* arg = value;
    switch (type.kind) {
        case TypeKind::Float:
            fmt = "%g\n";
            break;
        case TypeKind::Bool:
            fmt = "%d\n";
            arg = b.CreateZExt(value, llvm::Type::getInt32Ty(ctx_), "bool_ext");
            break;
        case TypeKind::String:
            fmt = "%s\n";
            break;
        case TypeKind::Int:
        default:
            fmt = "%lld\n";
            break;
    }

    llvm::Value* fmtStr = b.CreateGlobalStringPtr(fmt);
    b.CreateCall(printfFn, { fmtStr, arg });
}

bool CodeGenContext::emitObjectFile(const std::string& outputPath) {
    if (!verifyModule()) return false;

    optimizeModule();

    auto targetMachine = createTargetMachine();
    if (!targetMachine) return false;

    module_->setDataLayout(targetMachine->createDataLayout());

    std::error_code ec;
    llvm::raw_fd_ostream dest(outputPath, ec, llvm::sys::fs::OF_None);
    if (ec) return false;

    llvm::legacy::PassManager pass;
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::CGFT_ObjectFile)) {
        return false;
    }

    pass.run(*module_);
    dest.flush();
    return true;
}

bool CodeGenContext::emitIR(const std::string& outputPath) {
    if (!verifyModule()) return false;
    optimizeModule();

    std::error_code ec;
    llvm::raw_fd_ostream dest(outputPath, ec, llvm::sys::fs::OF_None);
    if (ec) return false;

    module_->print(dest, nullptr);
    dest.flush();
    return true;
}

bool CodeGenContext::emitExecutable(const std::string& outputPath) {
    std::string objPath = outputPath + ".o";
    if (!emitObjectFile(objPath)) return false;

    auto targetMachine = createTargetMachine();
    if (!targetMachine) return false;

    std::string linker = "cc";
    std::string cmd = linker + " -no-pie " + objPath + " -o " + outputPath;
    int result = std::system(cmd.c_str());
    std::remove(objPath.c_str());
    return result == 0;
}

} // namespace myl