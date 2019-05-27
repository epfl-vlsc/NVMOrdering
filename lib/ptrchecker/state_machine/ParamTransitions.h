#pragma once
#include "Common.h"
#include "Transitions.h"

namespace clang::ento::nvm {
class ParamDirtifier : public StoreManager ::BindingsHandler {
public:
  bool HandleBinding(StoreManager& SM, Store St, const MemRegion* Region,
                     SVal Val) {
    llvm::errs() << "lol\n";
    return true;
  }
};

} // namespace clang::ento::nvm::ParamTrans