#pragma once
#include "AnnotFunction.h"
#include "NamedFunctions.h"

namespace clang::ento::nvm {

class OrderFncs {
  static constexpr const char* PERSISTENT = "PersistentCode";
  static constexpr const char* RECOVERY = "RecoveryCode";
  static constexpr const char* SAFE = "SafeCode";

  AnnotFunction persistentFnc;
  AnnotFunction recoveryFnc;
  AnnotFunction safeFnc;
  VFenceFunction vfenceFnc;
  PFenceFunction pfenceFnc;
  FlushFunction flushFnc;
  FlushOptFunction flushOptFnc;
  NtiFunction ntiFnc;

public:
  OrderFncs()
      : persistentFnc(PERSISTENT), recoveryFnc(RECOVERY), safeFnc(SAFE) {}

  bool isPersistentFunction(CheckerContext& C) const {
    const FunctionDecl* FD = getFuncDecl(C);
    return persistentFnc.inFunctions(FD);
  }

  bool isRecoveryFunction(CheckerContext& C) const {
    const FunctionDecl* FD = getFuncDecl(C);
    return recoveryFnc.inFunctions(FD);
  }

  void insertIfKnown(const FunctionDecl* FD) {
    persistentFnc.insertIfKnown(FD);
    recoveryFnc.insertIfKnown(FD);
    safeFnc.insertIfKnown(FD);
    vfenceFnc.insertIfKnown(FD);
    pfenceFnc.insertIfKnown(FD);
    flushFnc.insertIfKnown(FD);
    flushOptFnc.insertIfKnown(FD);
    ntiFnc.insertIfKnown(FD);
  }

  bool isEndFunction(const CallEvent& Call) const {
    const FunctionDecl* FD = getFuncDecl(Call);
    return safeFnc.inFunctions(FD);
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
    safeFnc.dump();
    vfenceFnc.dump();
    pfenceFnc.dump();
    flushFnc.dump();
    flushOptFnc.dump();
    ntiFnc.dump();
  }
};

} // namespace clang::ento::nvm