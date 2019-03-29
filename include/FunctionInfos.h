#pragma once
#include "Common.h"
#include <set>

namespace clang::ento::nvm {

class BaseFunction {
protected:
  std::set<const FunctionDecl*> functions;

public:
  bool inFunctions(const FunctionDecl* D) const {
    return functions.count(D);
  }

  void dumpFunctions() {
    for (const FunctionDecl* FD : functions) {
      llvm::outs() << FD->getQualifiedNameAsString() << "\n";
    }
  }
};

class SpecialFunction : public BaseFunction {
public:
  virtual ~SpecialFunction(){}

  bool checkName(const FunctionDecl* FD) const {
    return checkName(FD->getIdentifier());
  }

  void insertIfKnown(const FunctionDecl* FD) {
    if (checkName(FD)) {
      functions.insert(FD);
    }
  }

  virtual bool checkName(const IdentifierInfo* II) const=0;
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
  AnnotFunction annotFnc;
  VFenceFunction vfenceFnc;
  PFenceFunction pfenceFnc;
  FlushFunction flushFnc;
  FlushOptFunction flushOptFnc;
  NtiFunction ntiFnc;

public:
  FunctionInfos(const char* annotation_):annotFnc(annotation_){}

  void insertIfKnown(const FunctionDecl* FD) {
    annotFnc.insertIfKnown(FD);
    vfenceFnc.insertIfKnown(FD);
    pfenceFnc.insertIfKnown(FD);
    flushFnc.insertIfKnown(FD);
    flushOptFnc.insertIfKnown(FD);
    ntiFnc.insertIfKnown(FD);
  }

  bool isAnnotatedFunction(CheckerContext& C) const {
    const FunctionDecl* FD = getFuncDecl(C);
    return annotFnc.inFunctions(FD);
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
};

} // namespace clang::ento::nvm