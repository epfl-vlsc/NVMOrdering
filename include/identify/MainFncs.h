#pragma once
#include "AnnotFunction.h"
#include "NamedFunctions.h"

namespace clang::ento::nvm {

class MainFncs {
  static constexpr const char* NVM_FNC = "NvmCode";
  static constexpr const char* SKIP_FNC = "SkipCode";

  AnnotFunction analyzeFnc;
  AnnotFunction skipFnc;
  PfenceFunction pfenceFnc;
  VfenceFunction vfenceFnc;
  FlushOptFunction flushOptFnc;
  FlushFenceFunction flushFenceFnc;

public:
  MainFncs() : analyzeFnc(NVM_FNC), skipFnc(SKIP_FNC) {}

  bool isSkip(const FunctionDecl* FD) const {
    return isFlushFenceFnc(FD) || isFlushOptFnc(FD) || isVfenceFnc(FD) ||
           isPfenceFnc(FD) || skipFnc.inFunctions(FD) || !isAnalyze(FD);
  }

  bool isAnalyze(const FunctionDecl* FD) const {
    return analyzeFnc.inFunctions(FD);
  }

  bool isSkip(CheckerContext& C) const {
    const FunctionDecl* FD = getFuncDecl(C);
    return isSkip(FD);
  }

  void insertAnalyze(const FunctionDecl* FD) { analyzeFnc.insert(FD); }

  void insertIfKnown(const FunctionDecl* FD) {
    analyzeFnc.insertIfKnown(FD);
    skipFnc.insertIfKnown(FD);
    pfenceFnc.insertIfKnown(FD);
    vfenceFnc.insertIfKnown(FD);
    flushOptFnc.insertIfKnown(FD);
    flushFenceFnc.insertIfKnown(FD);
  }

  void removeSkipFromAnalyze() {
    for (const FunctionDecl* FD : skipFnc) {
      analyzeFnc.erase(FD);
    }
  }

  bool isFlushFenceFnc(const FunctionDecl* FD) const {
    return flushFenceFnc.inFunctions(FD);
  }

  bool isFlushOptFnc(const FunctionDecl* FD) const {
    return flushOptFnc.inFunctions(FD);
  }

  bool isVfenceFnc(const FunctionDecl* FD) const {
    return vfenceFnc.inFunctions(FD);
  }

  bool isPfenceFnc(const FunctionDecl* FD) const {
    return pfenceFnc.inFunctions(FD);
  }

  void dump() const {
    printMsg("analyze:");
    analyzeFnc.dump();
    printMsg("use:");
    skipFnc.dump();
    flushFenceFnc.dump();
    flushOptFnc.dump();
    pfenceFnc.dump();
    vfenceFnc.dump();
  }

  auto begin() const { return analyzeFnc.begin(); }

  auto end() const { return analyzeFnc.end(); }
};

} // namespace clang::ento::nvm