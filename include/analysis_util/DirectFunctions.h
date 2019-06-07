#pragma once
#include "AnnotFunction.h"
#include "DirectAnalysis.h"
#include "NamedFunctions.h"

namespace clang::ento::nvm {

class DirectFunctions {
public:
  using FunctionInfo = typename DirectAnalysis::FunctionInfo;
  using VarSet = typename DirectAnalysis::VarSet;
  using FuncSet = typename DirectAnalysis::FuncSet;

private:
  static constexpr const char* NVM_FNC = "NvmCode";
  static constexpr const char* SKIP_FNC = "SkipCode";

  AnnotFunction analyzedFnc;
  AnnotFunction skipFnc;
  PfenceFunction pfenceFnc;
  VfenceFunction vfenceFnc;
  FlushOptFunction flushOptFnc;
  FlushFenceFunction flushFenceFnc;

  DirectAnalysis allUnitsInfo;

public:
  DirectFunctions() : analyzedFnc(NVM_FNC), skipFnc(SKIP_FNC) {}

  void addUnitInfo(const FunctionDecl* FD, VarSet& usedVars,
                   FuncSet& usedFuncs) {
    allUnitsInfo.addFunctionInfo(FD, usedVars, usedFuncs);
  }

  auto& getUnitInfo(const FunctionDecl* FD) {
    return allUnitsInfo.getFunctionInfo(FD);
  }

  bool isSkipFunction(const FunctionDecl* FD) const {
    return isFlushFenceFunction(FD) || isFlushOptFunction(FD) ||
           isVfenceFunction(FD) || isPfenceFunction(FD) ||
           skipFnc.inFunctions(FD);
  }

  bool isAnalyzedFunction(const FunctionDecl* FD) const {
    return analyzedFnc.inFunctions(FD);
  }

  bool isSkipFunction(CheckerContext& C) const {
    const FunctionDecl* FD = getFuncDecl(C);
    return isSkipFunction(FD);
  }

  void insertAnalyzeFunction(const FunctionDecl* FD) { analyzedFnc.insert(FD); }

  void insertIfKnown(const FunctionDecl* FD) {
    analyzedFnc.insertIfKnown(FD);
    skipFnc.insertIfKnown(FD);
    pfenceFnc.insertIfKnown(FD);
    vfenceFnc.insertIfKnown(FD);
    flushOptFnc.insertIfKnown(FD);
    flushFenceFnc.insertIfKnown(FD);
  }

  void removeSkipFromAnalyze() {
    for (const FunctionDecl* FD : skipFnc) {
      analyzedFnc.erase(FD);
    }
  }

  bool isFlushFenceFunction(const FunctionDecl* FD) const {
    return flushFenceFnc.inFunctions(FD);
  }

  bool isFlushOptFunction(const FunctionDecl* FD) const {
    return flushOptFnc.inFunctions(FD);
  }

  bool isVfenceFunction(const FunctionDecl* FD) const {
    return vfenceFnc.inFunctions(FD);
  }

  bool isPfenceFunction(const FunctionDecl* FD) const {
    return pfenceFnc.inFunctions(FD);
  }

  void dump() const {
    printMsg("analyze:");
    analyzedFnc.dump();
    printMsg("use:");
    skipFnc.dump();
    flushFenceFnc.dump();
    flushOptFnc.dump();
    pfenceFnc.dump();
    vfenceFnc.dump();
  }

  auto begin() const { return analyzedFnc.begin(); }

  auto end() const { return analyzedFnc.end(); }
};

} // namespace clang::ento::nvm