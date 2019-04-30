#pragma once
#include "identify/AnnotFunction.h"
#include "identify/NamedFunctions.h"

namespace clang::ento::nvm {

class LogFncs {
  static constexpr const char* ANALYZE = "LogCode";
  static constexpr const char* LOGFNC = "LogFnc";

  AnnotFunction analysisFnc;
  AnnotFunction logFnc;

public:
  LogFncs() : analysisFnc(ANALYZE), logFnc(LOGFNC) {}

  bool isAnalysisFunction(const FunctionDecl* FD) const {
    return analysisFnc.inFunctions(FD);
  }

  bool isLogFunction(const FunctionDecl* FD) const {
    return logFnc.inFunctions(FD);
  }

  void insertIfKnown(const FunctionDecl* FD) {
    analysisFnc.insertIfKnown(FD);
    logFnc.insertIfKnown(FD);
  }

  void dump() {
    analysisFnc.dump();
    logFnc.dump();
  }
};

} // namespace clang::ento::nvm