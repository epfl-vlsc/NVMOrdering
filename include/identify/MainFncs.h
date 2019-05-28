#pragma once
#include "identify/AnnotFunction.h"
#include "identify/NamedFunctions.h"

namespace clang::ento::nvm {

class MainFncs {
  static constexpr const char* SKIP_FNC = "SkipCode";

  BaseFunction analyzeFnc;
  AnnotFunction skipFnc;
  PfenceFunction pfenceFnc;
  VfenceFunction vfenceFnc;
  FlushOptFunction flushOptFnc;
  FlushFenceFunction flushFenceFnc;

public:
  MainFncs() : skipFnc(SKIP_FNC) {}

  bool isSkip(const FunctionDecl* FD) const {
    return isFlushFenceFnc(FD) || isFlushOptFnc(FD) || isVfenceFnc(FD) ||
           isPfenceFnc(FD) || skipFnc.inFunctions(FD) ||
           !analyzeFnc.inFunctions(FD);
  }

  bool isSkip(CheckerContext& C) const {
    const FunctionDecl* FD = getFuncDecl(C);
    return isSkip(FD);
  }

  void insertAnalyze(const FunctionDecl* FD) { analyzeFnc.insert(FD); }

  void insertIfKnown(const FunctionDecl* FD) {
    skipFnc.insertIfKnown(FD);
    pfenceFnc.insertIfKnown(FD);
    vfenceFnc.insertIfKnown(FD);
    flushOptFnc.insertIfKnown(FD);
    flushFenceFnc.insertIfKnown(FD);
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

  void dump() {
    printMsg("analyze:");
    analyzeFnc.dump();
    printMsg("use:");
    skipFnc.dump();
    flushFenceFnc.dump();
    flushOptFnc.dump();
    pfenceFnc.dump();
    vfenceFnc.dump();
  }
};

} // namespace clang::ento::nvm