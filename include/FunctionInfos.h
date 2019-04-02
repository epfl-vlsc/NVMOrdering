#pragma once
#include "Common.h"
#include <set>

namespace clang::ento::nvm {

class BaseFunction {
protected:
  std::set<const FunctionDecl*> functions;

public:
  bool inFunctions(const FunctionDecl* D) const { return functions.count(D); }

  void dump() {
    for (const FunctionDecl* FD : functions) {
      llvm::outs() << FD->getQualifiedNameAsString() << "\n";
    }
  }
};

class SpecialFunction : public BaseFunction {
public:
  virtual ~SpecialFunction() {}

  bool checkName(const FunctionDecl* FD) const {
    return checkName(FD->getIdentifier());
  }

  void insertIfKnown(const FunctionDecl* FD) {
    if (checkName(FD)) {
      functions.insert(FD);
    }
  }

  virtual bool checkName(const IdentifierInfo* II) const = 0;
};

class FlushFunction : public SpecialFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && II->isStr("clflush");
  }
};

class FlushOptFunction : public SpecialFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && (II->isStr("clflushopt") || II->isStr("clwb"));
  }
};

class NtiFunction : public SpecialFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && II->isStr("nti");
  }
};

class PFenceFunction : public SpecialFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && II->isStr("pfence");
  }
};

class VFenceFunction : public SpecialFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && II->isStr("vfence");
  }
};

class AnnotFunction : public BaseFunction {
  const char* annotation;

public:
  AnnotFunction(const char* annotation_) : annotation(annotation_) {}

  bool checkName(const AnnotateAttr* AA) const {
    return AA && AA->getAnnotation() == annotation;
  }

  void insertIfKnown(const FunctionDecl* D) {
    // if has attribute for analysis, add function to set
    for (const auto* Ann : D->specific_attrs<AnnotateAttr>()) {
      if (checkName(Ann)) {
        functions.insert(D);
      }
    }
  }
};

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