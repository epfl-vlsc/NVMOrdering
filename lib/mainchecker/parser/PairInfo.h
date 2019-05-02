#pragma once

#include "Common.h"

namespace clang::ento::nvm {

class PairInfo {
  const NamedDecl* data;
  const NamedDecl* check;

  bool isScl;

public:
  PairInfo(const NamedDecl* data_, const NamedDecl* check_)
      : data(data_), check(check_), isScl(false) {
    assert(data && check);
  }

  PairInfo(const NamedDecl* data_, const NamedDecl* check_, bool isScl_)
      : data(data_), check(check_), isScl(isScl_) {
    assert(data && check);
  }

  void dump() const {
    assert(data && check);
    const char* clStr = (isScl) ? "scl" : "dcl";
    llvm::errs() << "<" << clStr << ":" << data->getNameAsString() << ","
                 << check->getNameAsString() << ">\n";
  }

  bool isData(const NamedDecl* ND) const { return data == ND; }

  bool isCheck(const NamedDecl* ND) const { return check == ND; }

  bool isSameCl() const { return isScl; }

  const NamedDecl* getDataND() const { return data; }

  const NamedDecl* getCheckND() const { return check; }
};

} // namespace clang::ento::nvm