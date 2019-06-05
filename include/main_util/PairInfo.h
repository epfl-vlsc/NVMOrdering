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

  std::string getNameAsString() const {
    assert(data && check);
    std::string sbuf;
    llvm::raw_string_ostream PairNameOs(sbuf);
    const char* clStr = (isScl) ? "scl" : "dcl";
    PairNameOs << "<" << clStr << ":" << data->getQualifiedNameAsString() << ","
               << check->getQualifiedNameAsString() << ">";

    return PairNameOs.str();
  }

  void dump() const { llvm::errs() << getNameAsString(); }

  bool isData(const NamedDecl* ND) const { return data == ND; }

  bool isCheck(const NamedDecl* ND) const { return check == ND; }

  bool isSameCl() const { return isScl; }

  const NamedDecl* getDataND() const { return data; }

  const NamedDecl* getCheckND() const { return check; }

  const NamedDecl* getPairND(const NamedDecl* ND) const {
    if (ND == data) {
      return check;
    } else if (ND == check) {
      return data;
    } else {
      llvm::report_fatal_error("not a member of pair");
      return nullptr;
    }
  }
};

} // namespace clang::ento::nvm