#pragma once
#include "AnnotFunction.h"
#include "NamedFunctions.h"

namespace clang::ento::nvm {

class DirectFunctions {
public:
  using FunctionSet = std::set<const FunctionDecl*>;

protected:
  static constexpr const char* NVM_FNC = "NvmCode";
  static constexpr const char* SKIP_FNC = "SkipCode";

  AnnotFunction analyzedFnc;
  AnnotFunction skipFnc;
  PfenceFunction pfenceFnc;
  VfenceFunction vfenceFnc;
  FlushFunction flushFnc;
  FlushFenceFunction flushFenceFnc;

public:
  DirectFunctions() : analyzedFnc(NVM_FNC), skipFnc(SKIP_FNC) {}

  auto& getAnalyzedFunctions() { return analyzedFnc; }

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

  void insertAnalyzeFunction(const FunctionDecl* FD) {
    analyzedFnc.insertFunctionDirect(FD);
  }

  void insertFunction(const FunctionDecl* FD) {
    analyzedFnc.insertFunction(FD);
    skipFnc.insertFunction(FD);
    pfenceFnc.insertFunction(FD);
    vfenceFnc.insertFunction(FD);
    flushFnc.insertFunction(FD);
    flushFenceFnc.insertFunction(FD);
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
    return flushFnc.inFunctions(FD);
  }

  bool isVfenceFunction(const FunctionDecl* FD) const {
    return vfenceFnc.inFunctions(FD);
  }

  bool isPfenceFunction(const FunctionDecl* FD) const {
    return pfenceFnc.inFunctions(FD);
  }

  void dumpFunctions() const {
    analyzedFnc.dump();
    skipFnc.dump();
    flushFenceFnc.dump();
    flushFnc.dump();
    pfenceFnc.dump();
    vfenceFnc.dump();
  }
};

} // namespace clang::ento::nvm