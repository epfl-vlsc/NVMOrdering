#pragma once
#include "identify/AnnotVar.h"

namespace clang::ento::nvm {

class LogVars {
  static constexpr const char* PERSIST = "persist";

  AnnotVar persistField;

public:
  LogVars() : persistField(PERSIST) {}

  bool isUsedVar(const NamedDecl* ND) const {
    return persistField.inValues(ND);
  }

  void insertIfKnown(const NamedDecl* ND) {
    persistField.insertIfKnown(ND);
  }

  void dump() {
    persistField.dump();
  }
};

} // namespace clang::ento::nvm