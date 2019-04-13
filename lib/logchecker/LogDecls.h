#pragma once
#include "Common.h"
#include "AnnotFunction.h"
#include "TypeInfos.h"

namespace clang::ento::nvm {

class LogDecls {
  static constexpr const char* PTR = "LogPtr";
  static constexpr const char* CODE = "LogCode";
  static constexpr const char* REC = "RecoveryCode";
  static constexpr const char* LOG = "log";

  AnnotFunction ptrFnc;
  AnnotFunction codeFnc;
  AnnotFunction recFnc;

  AnnotVar logVar;

public:
  LogDecls() : ptrFnc(PTR), codeFnc(CODE), recFnc(REC), logVar(LOG) {}

  void insertIfKnown(const FunctionDecl* FD) {
    ptrFnc.insertIfKnown(FD);
    codeFnc.insertIfKnown(FD);
    recFnc.insertIfKnown(FD);
  }

  void insertIfKnown(const ValueDecl* VD) { logVar.insertIfKnown(VD); }

  bool isPtrFunction(const CallEvent& Call) const {
    const FunctionDecl* FD = getFuncDecl(Call);
    return ptrFnc.inFunctions(FD);
  }

  bool isCodeFunction(const CallEvent& Call) const {
    const FunctionDecl* FD = getFuncDecl(Call);
    return codeFnc.inFunctions(FD);
  }

  bool isRecoveryFunction(const FunctionDecl* FD) const {
    return recFnc.inFunctions(FD);
  }

  bool islogVar(const ValueDecl* VD) const {
    return logVar.inValues(VD);
  }

  void dump() {
    ptrFnc.dump();
    codeFnc.dump();
    recFnc.dump();
    logVar.dump();
  }
};

} // namespace clang::ento::nvm