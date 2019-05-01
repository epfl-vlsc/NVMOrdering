#pragma once

#include "Common.h"

namespace clang::ento::nvm {

class PairInfo {
  const NamedDecl* data;
  const NamedDecl* check;

public:
  PairInfo(const NamedDecl* data_, const NamedDecl* check_)
      : data(data_), check(check_) {
    assert(data && check);
  }

  void dump() const {
    assert(data && check);
    llvm::errs() << "<" << data->getNameAsString() << ","
                 << check->getNameAsString() << ">\n";
  }

  bool isData(const NamedDecl* ND) { return data == ND; }

  bool isCheck(const NamedDecl* ND) { return check == ND; }
};

} // namespace clang::ento::nvm