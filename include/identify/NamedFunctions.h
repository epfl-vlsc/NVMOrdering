#pragma once
#include "BaseFunction.h"

namespace clang::ento::nvm {

class NamedFunction : public BaseFunction {
public:
  virtual ~NamedFunction() {}

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

class FlushFunction : public NamedFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && II->isStr("clflush");
  }
};

class FlushOptFunction : public NamedFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && (II->isStr("clflushopt") || II->isStr("clwb"));
  }
};

class NtiFunction : public NamedFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && II->isStr("nti");
  }
};

class PFenceFunction : public NamedFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && II->isStr("pfence");
  }
};

class VFenceFunction : public NamedFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && II->isStr("vfence");
  }
};

} // namespace clang::ento::nvm