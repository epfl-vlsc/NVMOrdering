#pragma once
#include "Common.h"
#include <set>

namespace clang::ento::nvm {

class BaseFunction {
protected:
  std::set<const FunctionDecl*> functions;

public:
  bool inFunctions(const FunctionDecl* D) const { return functions.count(D); }

  void dump() {
    for (const FunctionDecl* FD : functions) {
      llvm::outs() << FD->getQualifiedNameAsString() << "\n";
    }
  }
};

} // namespace clang::ento::nvm