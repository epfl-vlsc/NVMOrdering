#pragma once
#include "AnnotFunctionInfos.h"
#include "SpecialFunctionInfos.h"

namespace clang::ento::nvm {

class FunctionInfos {
  static constexpr const char* PERSISTENT = "PersistentCode";
  static constexpr const char* RECOVERY = "RecoveryCode";
  static constexpr const char* END = "EndCode";

  AnnotFunction persistentFnc;
  AnnotFunction recoveryFnc;
  AnnotFunction endFnc;
  VFenceFunction vfenceFnc;
  PFenceFunction pfenceFnc;
  FlushFunction flushFnc;
  FlushOptFunction flushOptFnc;
  NtiFunction ntiFnc;

public:
  FunctionInfos() : persistentFnc(PERSISTENT), recoveryFnc(RECOVERY), endFnc(END) {}

  void insertIfKnown(const FunctionDecl* FD) {
    persistentFnc.insertIfKnown(FD);
    recoveryFnc.insertIfKnown(FD);
    endFnc.insertIfKnown(FD);
    vfenceFnc.insertIfKnown(FD);
    pfenceFnc.insertIfKnown(FD);
    flushFnc.insertIfKnown(FD);
    flushOptFnc.insertIfKnown(FD);
    ntiFnc.insertIfKnown(FD);
  }

  bool isPersistentFunction(CheckerContext& C) const {
    const FunctionDecl* FD = getFuncDecl(C);
    return persistentFnc.inFunctions(FD);
  }

  bool isRecoveryFunction(CheckerContext& C) const {
    const FunctionDecl* FD = getFuncDecl(C);
    return recoveryFnc.inFunctions(FD);
  }

  bool isEndFunction(const CallEvent& Call) const {
    const FunctionDecl* FD = getFuncDecl(Call);
    return endFnc.inFunctions(FD);
  }

  bool isFlushFunction(const CallEvent& Call) const {
    const FunctionDecl* FD = getFuncDecl(Call);
    return flushFnc.inFunctions(FD);
  }

  bool isPFenceFunction(const CallEvent& Call) const {
    const FunctionDecl* FD = getFuncDecl(Call);
    return pfenceFnc.inFunctions(FD);
  }

  bool isVFenceFunction(const CallEvent& Call) const {
    const FunctionDecl* FD = getFuncDecl(Call);
    return vfenceFnc.inFunctions(FD);
  }

  void dump() {
    persistentFnc.dump();
    recoveryFnc.dump();
    endFnc.dump();
    vfenceFnc.dump();
    pfenceFnc.dump();
    flushFnc.dump();
    flushOptFnc.dump();
    ntiFnc.dump();
  }
};

} // namespace clang::ento::nvm