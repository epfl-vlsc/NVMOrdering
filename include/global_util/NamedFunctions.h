#pragma once
#include "BaseFunction.h"

namespace clang::ento::nvm {

class NamedFunction : public BaseFunction {
public:
  virtual ~NamedFunction() {}

  bool checkName(const FunctionDecl* FD) const {
    return checkName(FD->getIdentifier());
  }

  void insertFunction(const FunctionDecl* FD) {
    if (checkName(FD)) {
      functions.insert(FD);
    }
  }

  void dump() const{
    llvm::errs() << getName() << " ";
    BaseFunction::dump();
  }

  virtual const char* getName() const = 0;
  virtual bool checkName(const IdentifierInfo* II) const = 0;
};

class FlushFunction : public NamedFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && (II->isStr("clflushopt") || II->isStr("clwb"));
  }

  const char* getName() const { return "flush"; }
};

class NtiFunction : public NamedFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && II->isStr("nti");
  }

  const char* getName() const { return "nti"; }
};

class PfenceFunction : public NamedFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && II->isStr("pfence");
  }

  const char* getName() const { return "pfence"; }
};

class VfenceFunction : public NamedFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && II->isStr("vfence");
  }

  const char* getName() const { return "vfence"; }
};

class FlushFenceFunction : public NamedFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && (II->isStr("flush_range") || II->isStr("clflush") ||
                  II->isStr("__pmem_persist"));
  }

  const char* getName() const { return "flushfence"; }
};

class LogFunction : public NamedFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && (II->isStr("log") || II->isStr("log_range"));
  }

  const char* getName() const { return "log"; }
};

class TxBegFunction : public NamedFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && (II->isStr("tx_begin"));
  }

  const char* getName() const { return "tx_beg"; }
};

class TxEndFunction : public NamedFunction {
public:
  bool checkName(const IdentifierInfo* II) const {
    return II && (II->isStr("tx_end"));
  }

  const char* getName() const { return "tx_end"; }
};

} // namespace clang::ento::nvm