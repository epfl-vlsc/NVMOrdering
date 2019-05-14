#pragma once
#include "identify/AnnotVar.h"

namespace clang::ento::nvm {

class LogVars {
  static constexpr const char* LOG = "log";

  AnnotVar logField;

public:
  LogVars() : logField(LOG) {}

  bool isUsedVar(const NamedDecl* ND) const {
    return logField.inValues(ND);
  }

  void insertIfKnown(const NamedDecl* ND) {
    logField.insertIfKnown(ND);
  }

  void dump() {
    logField.dump();
  }
};

} // namespace clang::ento::nvm