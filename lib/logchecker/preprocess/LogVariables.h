#pragma once
#include "Common.h"
#include "global_util/AnnotVariable.h"

namespace clang::ento::nvm {

class LogVariables {
public:
  using LatVar = const NamedDecl*;
  using VariableSet = std::set<LatVar>;

private:
  static constexpr const char* LOG = "LogField";

  AnnotVariable logField;

public:
  LogVariables() : logField(LOG) {}

  bool isUsedVar(LatVar var) const { return logField.isUsedVar(var); }

  void insertVariable(LatVar var) { logField.insertVariable(var); }

  void dumpVariables() const { logField.dump(); }
};

} // namespace clang::ento::nvm