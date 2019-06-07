#pragma once
#include "AnnotFunction.h"
#include "NamedFunctions.h"

namespace clang::ento::nvm {

class DirectAnalysis {

  template<typename VS, typename FS>
  class FunctionInfoT {
    VS usedVars;
    FS usedFuncs;

  public:
    VS& getUsedVars() { return usedVars; }
    FS& getUsedFuncs() { return usedFuncs; }

    FunctionInfoT(VS& usedVars_, FS& usedFuncs_) {
      usedVars = std::move(usedVars_);
      usedFuncs = std::move(usedFuncs_);
    }

    FunctionInfoT() {}

    bool isUsedVar(const NamedDecl* ND) const {
      assert(ND);
      return usedVars.count(ND);
    }

    bool isUsedFunc(const FunctionDecl* FD) const {
      assert(FD);
      return usedFuncs.count(FD);
    }
  };

public:
  using VarSet = std::set<const NamedDecl*>;
  using FuncSet = std::set<const FunctionDecl*>;
  using FunctionInfo = FunctionInfoT<VarSet, FuncSet>;
  using FunctionInfoMap = std::map<const FunctionDecl*, FunctionInfo>;

private:
  FunctionInfoMap functionInfoMap;

public:
  FunctionInfo& getFunctionInfo(const FunctionDecl* FD) {
    return functionInfoMap[FD];
  }

  void addFunctionInfo(const FunctionDecl* FD, VarSet& usedVars,
                       FuncSet& usedFuncs) {
    FunctionInfo functionInfo(usedVars, usedFuncs);
    functionInfoMap[FD] = functionInfo;
  }
};

} // namespace clang::ento::nvm