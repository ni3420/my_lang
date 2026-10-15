#pragma once

#include "myl/ast.hpp"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <string>
#include <unordered_map>
#include <memory>

namespace myl {

class CodeGenContext {
public:
    CodeGenContext();
    ~CodeGenContext();

    llvm::LLVMContext& context() { return ctx_; }
    llvm::Module& module() { return *module_; }
    llvm::IRBuilder<>& builder() { return *builder_; }

    llvm::Type* llvmType(Type type);
    llvm::Function* getFunction(const std::string& name);
    void setFunction(const std::string& name, llvm::Function* fn);
    llvm::Value* getVariable(const std::string& name);
    void setVariable(const std::string& name, llvm::Value* val);

    // Emit a call to the C runtime to print a single value.
    void emitPrint(llvm::Value* value, Type type);

    bool verifyModule();
    void optimizeModule();
    bool emitObjectFile(const std::string& outputPath);
    bool emitIR(const std::string& outputPath);
    bool emitExecutable(const std::string& outputPath);

private:
    llvm::LLVMContext ctx_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;
    std::unique_ptr<llvm::legacy::FunctionPassManager> fpm_;
    std::unordered_map<std::string, llvm::Function*> functions_;
    std::unordered_map<std::string, llvm::Value*> variables_;

    void initializeModule();
    void initializePassManager();
    llvm::TargetMachine* createTargetMachine();
};

} // namespace myl