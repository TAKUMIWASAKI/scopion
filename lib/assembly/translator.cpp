#include "scopion/assembly/scope.hpp"
#include "scopion/assembly/translator.hpp"

#include "scopion/error.hpp"

#include <algorithm>

#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/iterator_range.hpp>

namespace scopion {
namespace assembly {

translator::translator(std::shared_ptr<llvm::Module> &module,
                       llvm::IRBuilder<> const &builder,
                       std::string const &code)
    : boost::static_visitor<llvm::Value *>(), module_(module),
      builder_(builder),
      code_range_(boost::make_iterator_range(code.begin(), code.end())) {
  {
    std::vector<llvm::Type *> args = {builder_.getInt8Ty()->getPointerTo()};

    module_->getOrInsertFunction(
        "printf",
        llvm::FunctionType::get(builder_.getInt32Ty(),
                                llvm::ArrayRef<llvm::Type *>(args), true));
    module_->getOrInsertFunction(
        "puts",
        llvm::FunctionType::get(builder_.getInt32Ty(),
                                llvm::ArrayRef<llvm::Type *>(args), true));
  }
  {
    std::vector<llvm::Type *> func_args_type;
    func_args_type.push_back(builder_.getInt32Ty());

    llvm::FunctionType *llvm_func_type = llvm::FunctionType::get(
        builder_.getInt32Ty(), func_args_type, /*可変長引数=*/false);
    llvm::Function *llvm_func =
        llvm::Function::Create(llvm_func_type, llvm::Function::ExternalLinkage,
                               "printn", module_.get());

    llvm::BasicBlock *entry =
        llvm::BasicBlock::Create(module_->getContext(),
                                 "entry_printn", // BasicBlockの名前
                                 llvm_func);
    currentScope_ = std::unique_ptr<scope>(new scope(entry));
    builder_.SetInsertPoint(entry); // entryの開始
    std::vector<llvm::Value *> args = {
        builder_.CreateGlobalStringPtr("%d\n"),
        &(*(llvm_func->getArgumentList().begin()))};
    builder_.CreateCall(module_->getFunction("printf"), args);
    builder_.CreateRet(llvm::ConstantInt::get(builder_.getInt32Ty(), 0, true));
  }
}

llvm::Value *translator::operator()(ast::value value) {
  return boost::apply_visitor(*this, value);
}

llvm::Value *translator::operator()(ast::operators value) {
  return boost::apply_visitor(*this, value);
}

llvm::Value *translator::operator()(ast::integer value) {
  if (ast::attr(value).lval)
    throw error("An integer constant is not to be assigned",
                ast::attr(value).where, code_range_);

  if (ast::attr(value).to_call)
    throw error("An integer constant is not to be called",
                ast::attr(value).where, code_range_);

  return llvm::ConstantInt::get(builder_.getInt32Ty(), ast::val(value));
}

llvm::Value *translator::operator()(ast::boolean value) {
  if (ast::attr(value).lval)
    throw error("A boolean constant is not to be assigned",
                ast::attr(value).where, code_range_);

  if (ast::attr(value).to_call)
    throw error("A boolean constant is not to be called",
                ast::attr(value).where, code_range_);

  return llvm::ConstantInt::get(builder_.getInt1Ty(), ast::val(value));
}

llvm::Value *translator::operator()(ast::string const &value) {
  if (ast::attr(value).lval)
    throw error("A string constant is not to be assigned",
                ast::attr(value).where, code_range_);

  if (ast::attr(value).to_call)
    throw error("A string constant is not to be called", ast::attr(value).where,
                code_range_);

  return builder_.CreateGlobalStringPtr(ast::val(value));
}

llvm::Value *translator::operator()(ast::variable const &value) {
  if (ast::attr(value).to_call) {
    auto *valp = module_->getFunction(ast::val(value));
    if (valp != nullptr) {
      return valp;
    } else {
      try {
        auto *varp = currentScope_->symbols.at(ast::val(value));

        if (varp->getType()->getPointerElementType()->isPointerTy()) {
          if (varp->getType()
                  ->getPointerElementType()
                  ->getPointerElementType()
                  ->isFunctionTy()) {
            return builder_.CreateLoad(varp);
          }
        }
        if (varp->getType()->getPointerElementType()->isLabelTy())
          return builder_.CreateLoad(varp);
        throw error("Variable \"" + ast::val(value) +
                        "\" is not a function but " +
                        getNameString(varp->getType()->getPointerElementType()),
                    ast::attr(value).where, code_range_);
      } catch (std::out_of_range &) {
        throw error("Function \"" + ast::val(value) +
                        "\" has not declared in this scope",
                    ast::attr(value).where, code_range_);
      }
    }
  } else {
    try {
      auto *valp = currentScope_->symbols.at(ast::val(value));
      if (ast::attr(value).lval) {
        return valp;
      } else {
        return builder_.CreateLoad(valp);
      }
    } catch (std::out_of_range &) {
      if (ast::attr(value).lval) {
        return nullptr;
      } else {
        throw error("\"" + ast::val(value) +
                        "\" has not declared in this scope",
                    ast::attr(value).where, code_range_);
      }
    }
  }
  assert(false);
}
llvm::Value *translator::operator()(ast::array const &value) {
  if (ast::attr(value).lval)
    throw error("An array constant is not to be assigned",
                ast::attr(value).where, code_range_);

  if (ast::attr(value).to_call)
    throw error("An array constant is not to be called", ast::attr(value).where,
                code_range_);

  auto &ary = ast::val(value);
  std::vector<llvm::Value *> values;
  for (auto const &elm : ary) {
    // Convert exprs to llvm::Value* and store it into new vector
    values.push_back(boost::apply_visitor(*this, elm));
  }

  auto t = values.empty() ? builder_.getVoidTy() : values[0]->getType();
  // check if the type of all elements of array is same
  if (!std::all_of(values.begin(), values.end(),
                   [&t](auto v) { return t == v->getType(); }))
    throw error("all elements of array must have the same type",
                ast::attr(value).where, code_range_);

  auto aryType = llvm::ArrayType::get(t, ary.size());
  auto aryPtr = builder_.CreateAlloca(aryType); // Allocate necessary memory
  for (auto const &v : values | boost::adaptors::indexed()) {
    std::vector<llvm::Value *> idxList = {builder_.getInt32(0),
                                          builder_.getInt32(v.index())};
    auto p = builder_.CreateInBoundsGEP(aryType, aryPtr,
                                        llvm::ArrayRef<llvm::Value *>(idxList));

    builder_.CreateStore(v.value(), p);
  }
  return aryPtr;
}

llvm::Value *translator::operator()(ast::arglist const &value) {
  return nullptr;
}

llvm::Value *translator::operator()(ast::function const &value) {
  if (ast::attr(value).lval)
    throw error("A function constant is not to be assigned",
                ast::attr(value).where, code_range_);

  auto &args = ast::val(value).first;
  auto &lines = ast::val(value).second;

  std::vector<llvm::Type *> func_args_type(args.size(), builder_.getInt32Ty());
  llvm::FunctionType *func_type =
      llvm::FunctionType::get(builder_.getVoidTy(), func_args_type, false);
  llvm::Function *func = llvm::Function::Create(
      func_type, llvm::Function::ExternalLinkage, "", module_.get());
  llvm::BasicBlock *entry =
      llvm::BasicBlock::Create(module_->getContext(),
                               "entryf", // BasicBlockの名前
                               func);
  auto prevScope = std::move(currentScope_);
  currentScope_ = std::unique_ptr<scope>(new scope(entry));

  auto *pb = builder_.GetInsertBlock();
  auto pp = builder_.GetInsertPoint();

  builder_.SetInsertPoint(entry); // entryの開始

  currentScope_->symbols["self"] =
      builder_.CreateAlloca(func->getType(), nullptr, "self");

  for (auto const &v : args) {
    currentScope_->symbols[ast::val(v)] =
        builder_.CreateAlloca(builder_.getInt32Ty(), nullptr,
                              ast::val(v)); // declare arguments
  }

  for (auto const &line : lines) {
    boost::apply_visitor(*this,
                         line); // If lines has function definition, the
                                // function will be defined twice.
  }

  llvm::Type *ret_type = nullptr;
  for (auto itr = entry->getInstList().begin();
       itr != entry->getInstList().end(); ++itr) {
    if ((*itr).getOpcode() == llvm::Instruction::Ret) {
      if ((*itr).getOperand(0)->getType() != ret_type) {
        if (ret_type == nullptr) {
          ret_type = (*itr).getOperand(0)->getType();
        } else {
          throw error("All return values must have the same type",
                      ast::attr(value).where, code_range_);
        }
      }
    }
  }
  if (ret_type == nullptr) {
    builder_.CreateRetVoid();
    ret_type = builder_.getVoidTy();
  }

  func->eraseFromParent(); // remove old one

  std::vector<llvm::Type *> args_type = {builder_.getInt32Ty()};
  llvm::Function *newfunc = llvm::Function::Create(
      llvm::FunctionType::get(ret_type, func_args_type, false),
      llvm::Function::ExternalLinkage, "", module_.get());
  llvm::BasicBlock *newentry =
      llvm::BasicBlock::Create(module_->getContext(),
                               "entry", // BasicBlockの名前
                               newfunc);
  currentScope_ = std::unique_ptr<scope>(new scope(newentry));
  builder_.SetInsertPoint(newentry);

  auto selfptr = builder_.CreateAlloca(newfunc->getType(), nullptr, "self");
  currentScope_->symbols["self"] = selfptr;
  builder_.CreateStore(newfunc, selfptr);

  auto it = newfunc->getArgumentList().begin();
  for (auto const &v : args) {
    auto aptr = builder_.CreateAlloca(builder_.getInt32Ty(), nullptr,
                                      ast::val(v)); // declare arguments
    currentScope_->symbols[ast::val(v)] = aptr;
    builder_.CreateStore(&(*it), aptr);
    it++;
  }

  for (auto const &line : lines) {
    boost::apply_visitor(*this, line);
  }
  if (ret_type == builder_.getVoidTy()) {
    builder_.CreateRetVoid();
  }

  currentScope_ = std::move(prevScope);
  builder_.SetInsertPoint(pb, pp); // entryを抜ける

  return newfunc;
}

llvm::Value *translator::operator()(ast::scope const &value) {
  auto sc = std::unique_ptr<scope>(new scope(llvm::BasicBlock::Create(
      module_->getContext(), "", builder_.GetInsertBlock()->getParent())));

  std::copy(currentScope_->symbols.begin(), currentScope_->symbols.end(),
            std::inserter(sc->symbols, sc->symbols.end()));
  auto prevScope = std::move(currentScope_);
  currentScope_ = std::move(sc);

  builder_.CreateBr(currentScope_->block);

  builder_.SetInsertPoint(currentScope_->block);

  for (auto const &line : ast::val(value)) {
    boost::apply_visitor(*this, line);
  }

  auto cs = std::move(currentScope_);
  currentScope_ = std::move(prevScope);

  return cs->block;
}

}; // namespace assembly
}; // namespace scopion
