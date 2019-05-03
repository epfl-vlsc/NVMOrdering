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

class PfenceFunction : public NamedFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && II->isStr("pfence");
  }
};

class VfenceFunction : public NamedFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && II->isStr("vfence");
  }
};

class FlushFenceFunction : public NamedFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && II->isStr("flush_range");
  }
};

} // namespace clang::ento::nvm