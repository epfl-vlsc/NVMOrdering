#pragma once
#include "Common.h"
#include "FunctionInfos.h"
#include "TypeInfos.h"

namespace clang::ento::nvm {

class LogInfos {
  static constexpr const char* PTR = "LogPtr";
  static constexpr const char* CODE = "LogCode";
  static constexpr const char* LOG = "log";

  AnnotFunction ptrFnc;
  AnnotFunction codeFnc;

  SpecialValue specialValue;

public:
  LogInfos() : ptrFnc(PTR), codeFnc(CODE), specialValue(LOG) {}

  void insertIfKnown(const FunctionDecl* FD) {
    ptrFnc.insertIfKnown(FD);
    codeFnc.insertIfKnown(FD);
  }

  void insertIfKnown(const ValueDecl* VD) {
    specialValue.insertIfKnown(VD);
  }

  bool isPtrFunction(const CallEvent& Call) const {
    const FunctionDecl* FD = getFuncDecl(Call);
    return ptrFnc.inFunctions(FD);
  }

  bool isCodeFunction(const CallEvent& Call) const {
    const FunctionDecl* FD = getFuncDecl(Call);
    return codeFnc.inFunctions(FD);
  }

  bool isSpecialValue(const ValueDecl* VD) const {
    return specialValue.inValues(VD);
  }

  void dump() {
    ptrFnc.dump();
    codeFnc.dump();
    specialValue.dump();
  }
};

} // namespace clang::ento::nvm