#pragma once
#include "Common.h"
#include "FunctionInfos.h"
#include "TypeInfos.h"

namespace clang::ento::nvm {

class LogInfos {
  static constexpr const char* PTR = "LogPtr";
  static constexpr const char* CODE = "LogCode";
  static constexpr const char* REC = "RecoveryCode";
  static constexpr const char* LOG = "log";

  AnnotFunction ptrFnc;
  AnnotFunction codeFnc;
  AnnotFunction recFnc;

  SpecialValue specialValue;

public:
  LogInfos() : ptrFnc(PTR), codeFnc(CODE), recFnc(REC), specialValue(LOG) {}

  void insertIfKnown(const FunctionDecl* FD) {
    ptrFnc.insertIfKnown(FD);
    codeFnc.insertIfKnown(FD);
    recFnc.insertIfKnown(FD);
  }

  void insertIfKnown(const ValueDecl* VD) { specialValue.insertIfKnown(VD); }

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

  bool isSpecialValue(const ValueDecl* VD) const {
    return specialValue.inValues(VD);
  }

  void dump() {
    ptrFnc.dump();
    codeFnc.dump();
    recFnc.dump();
    specialValue.dump();
  }
};

} // namespace clang::ento::nvm