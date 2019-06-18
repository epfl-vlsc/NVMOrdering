#pragma once
#include "Common.h"
#include "global_util/AnnotVariable.h"

namespace clang::ento::nvm {

class FlushedVariables {
protected:
  using LatVar = const NamedDecl*;
  using VariableSet = std::set<LatVar>;

  static constexpr const char* FLUSHED = "FlushedPtr";

  AnnotVariable fieldVariables;
  VariableSet trackedVariables;

public:
  FlushedVariables() : fieldVariables(FLUSHED) {}

  bool isFieldVariable(LatVar var) const {
    return fieldVariables.count(var);
  }
  bool isTrackedVariable(LatVar var) const {
    return trackedVariables.count(var);
  }

  void insertFieldVariable(LatVar var) {
    fieldVariables.insertVariable(var);
  }

  void insertTrackedVariable(LatVar var) { trackedVariables.insert(var); }

  void dumpVariables() const {
    llvm::errs() << "vars:";

    llvm::errs() << " persistent pointers: ";
    for (auto& var : fieldVariables) {
      llvm::errs() << var->getQualifiedNameAsString() << ",";
    }

    llvm::errs() << " tracked variables: ";
    for (auto& var : trackedVariables) {
      llvm::errs() << var->getQualifiedNameAsString() << ",";
    }
    llvm::errs() << "\n";
  }
};
} // namespace clang::ento::nvm