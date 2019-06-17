#pragma once
#include "Common.h"

namespace clang::ento::nvm {

template <typename Variables, typename Functions>
class GlobalInfo : public Variables, public Functions {
  // Variables must define VariableSet
  // Functions must define FunctionSet

public:
  using LatVar = typename Variables::LatVar;
  using VariableSet = typename Variables::VariableSet;
  using FunctionSet = typename Functions::FunctionSet;

  class UnitInfo {
  private:
    VariableSet unitVariables;
    FunctionSet unitFunctions;

  public:
    bool isUsedVar(LatVar var) const {
      assert(var);
      return unitVariables.count(var);
    }

    bool isUsedFunc(const FunctionDecl* FD) const {
      assert(FD);
      return unitFunctions.count(FD);
    }

    VariableSet& getUnitVariables() { return unitVariables; }

    FunctionSet& getUnitFunctions() { return unitFunctions; }

    void insertVariable(LatVar var) { unitVariables.insert(var); }

    void insertFunction(const FunctionDecl* FD) { unitFunctions.insert(FD); }

    void dump() const {
      llvm::errs() << "unit variables: ";
      for (auto& var : unitVariables) {
        llvm::errs() << var->getQualifiedNameAsString() << ", ";
      }
      llvm::errs() << " | unit functions: ";
      for (auto& func : unitFunctions) {
        llvm::errs() << func->getQualifiedNameAsString() << ", ";
      }
      llvm::errs() << "\n";
    }
    //------------------------------------------
  };

private:
  using FunctionUnitMap = std::map<const FunctionDecl*, UnitInfo>;

  // unit based structures
  FunctionUnitMap functionUnitMap;
  UnitInfo* activeUnit;

public:
  ///--------------------
  void dump() const {
    Variables::dumpVariables();
    Functions::dumpFunctions();
  }

  void dumpUnit() const { activeUnit->dump(); }

  bool isUsedVarUnit(LatVar var) const {
    assert(activeUnit);
    return activeUnit->isUsedVar(var);
  }

  bool isUsedFuncUnit(const FunctionDecl* FD) const {
    assert(activeUnit);
    return activeUnit->isUsedFunc(FD);
  }

  void setActiveUnit(const FunctionDecl* FD) {
    assert(functionUnitMap.count(FD));
    activeUnit = &functionUnitMap[FD];
  }

  void initActiveUnit(const FunctionDecl* FD) {
    activeUnit = &functionUnitMap[FD];
  }

  void addVariableToActiveUnit(LatVar var) {
    assert(activeUnit);
    activeUnit->insertVariable(var);
  }

  void addFunctionToActiveUnit(const FunctionDecl* FD) {
    assert(activeUnit);
    activeUnit->insertFunction(FD);
  }

  auto& getUnitVariables() {
    assert(activeUnit);
    return activeUnit->getUnitVariables();
  }

  auto& getUnitFunctions() {
    assert(activeUnit);
    return activeUnit->getUnitFunctions();
  }

  const CallExpr* getCEIfAnalyzedFunction(const Stmt* S) const {
    if (const CallExpr* CE = dyn_cast<CallExpr>(S)) {
      const FunctionDecl* calleeFD = CE->getDirectCallee();
      if (!calleeFD)
        return nullptr;

      if (!this->isSkipFunction(calleeFD)) {
        return CE;
      }
    }

    return nullptr;
  }
};

} // namespace clang::ento::nvm