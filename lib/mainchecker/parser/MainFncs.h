#pragma once
#include "identify/AnnotFunction.h"
#include "identify/NamedFunctions.h"

namespace clang::ento::nvm {

class MainFncs {
  static constexpr const char* PERSISTENT = "PersistentCode";

  AnnotFunction persistentFnc;
  FlushFenceFunction flushFenceFnc;

public:
  MainFncs() : persistentFnc(PERSISTENT) {}

  bool isPersistentFunction(CheckerContext& C) const {
    const FunctionDecl* FD = getFuncDecl(C);
    return persistentFnc.inFunctions(FD);
  }

   void insertIfKnown(const FunctionDecl* FD) {
    persistentFnc.insertIfKnown(FD);
    flushFenceFnc.insertIfKnown(FD);
  }

  bool isFlushFenceFunction(const FunctionDecl* FD) const { 
    return flushFenceFnc.inFunctions(FD);
  }

  void dump() {
    persistentFnc.dump();
    flushFenceFnc.dump();
  }
};

} // namespace clang::ento::nvm