#pragma once
#include "identify/AnnotFunction.h"
#include "identify/NamedFunctions.h"

namespace clang::ento::nvm {

class LogFncs {
  static constexpr const char* LOG = "LogCode";

  AnnotFunction logFnc;

public:
  LogFncs() : logFnc(LOG) {}

  bool isLogFunction(const FunctionDecl* FD) const {
    return logFnc.inFunctions(FD);
  }

  void insertIfKnown(const FunctionDecl* FD) { logFnc.insertIfKnown(FD); }

  void dump() { logFnc.dump(); }
};

} // namespace clang::ento::nvm