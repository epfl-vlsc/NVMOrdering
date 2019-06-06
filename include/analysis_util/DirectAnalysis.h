#pragma once
#include "AnnotFunction.h"
#include "NamedFunctions.h"

namespace clang::ento::nvm {

class DirectAnalysis {
  using VarSet = std::set<const NamedDecl*>;
  using FuncSet = std::set<const FunctionDecl*>;

  class FunctionInfo {
    VarSet usedVars;
    FuncSet usedFuncs;

  public:
    VarSet& getUsedVars() { return usedVars; }
    FuncSet& usedFuncs() { return usedFuncs; }

    FunctionInfo(VarSet& usedVars_, FuncSet& usedFuncs_) {
      usedVars = std::move(usedVars_);
      usedFuncs = std::move(usedFuncs_);
    }
  };

  using FunctionInfoMap = std::map<const FunctionDecl*, FunctionInfo>;

  FunctionInfoMap functionInfoMap;

public:
  FunctionInfo& getFunctionInfo(const FunctionDecl* FD) {
    return functionInfoMap[FD];
  }

  void addFunctionInfo(const FunctionDecl* FD, VarSet& usedVars,
                       FuncSet& usedFuncs) {
    FunctionInfo functionInfo(usedVars, usedFuncs);
  }
};

} // namespace clang::ento::nvm