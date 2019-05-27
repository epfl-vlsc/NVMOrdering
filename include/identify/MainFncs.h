#pragma once
#include "identify/AnnotFunction.h"
#include "identify/NamedFunctions.h"

namespace clang::ento::nvm {

class MainFncs {
  static constexpr const char* SKIP_FNC = "SkipCode";

  AnnotFunction skipFnc;
  PfenceFunction pfenceFnc;
  VfenceFunction vfenceFnc;
  FlushOptFunction flushOptFnc;
  FlushFenceFunction flushFenceFnc;

public:
  MainFncs() : skipFnc(SKIP_FNC) {}

  bool isSkip(CheckerContext& C) const {
    if (const FunctionDecl* FD = getFuncDecl(C); FD) {
      return isSkip(FD);
    }
    return false;
  }

  bool isSkip(const FunctionDecl* FD) const {
    return isSkipFnc(FD) || isFlushFenceFnc(FD) || isFlushOptFnc(FD) ||
           isVfenceFnc(FD) || isPfenceFnc(FD);
  }

  void insertIfKnown(const FunctionDecl* FD) {
    skipFnc.insertIfKnown(FD);
    pfenceFnc.insertIfKnown(FD);
    vfenceFnc.insertIfKnown(FD);
    flushOptFnc.insertIfKnown(FD);
    flushFenceFnc.insertIfKnown(FD);
  }

  bool isSkipFnc(const FunctionDecl* FD) const {
    return skipFnc.inFunctions(FD);
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
    skipFnc.dump();
    flushFenceFnc.dump();
    flushOptFnc.dump();
    pfenceFnc.dump();
    vfenceFnc.dump();
  }
};

} // namespace clang::ento::nvm