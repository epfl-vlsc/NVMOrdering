#pragma once
#include "identify/AnnotFunction.h"
#include "identify/NamedFunctions.h"

namespace clang::ento::nvm {

class MainFncs {
  static constexpr const char* PERSISTENT = "PersistentCode";

  AnnotFunction persistentFnc;
  PfenceFunction pfenceFnc;
  VfenceFunction vfenceFnc;
  FlushOptFunction flushOptFnc;
  FlushFenceFunction flushFenceFnc;

public:
  MainFncs() : persistentFnc(PERSISTENT) {}

  bool isPersistentFunction(CheckerContext& C) const {
    const FunctionDecl* FD = getFuncDecl(C);
    return persistentFnc.inFunctions(FD);
  }

  void insertIfKnown(const FunctionDecl* FD) {
    persistentFnc.insertIfKnown(FD);
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

  bool isUsedFnc(const FunctionDecl* FD) const {
    return isFlushFenceFnc(FD) || isFlushOptFnc(FD) || isVfenceFnc(FD) ||
           isPfenceFnc(FD);
  }

  void dump() {
    persistentFnc.dump();
    flushFenceFnc.dump();
    flushOptFnc.dump();
    pfenceFnc.dump();
    vfenceFnc.dump();
  }
};

} // namespace clang::ento::nvm