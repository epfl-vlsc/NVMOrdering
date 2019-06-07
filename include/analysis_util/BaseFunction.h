#pragma once
#include "Common.h"
#include <set>

namespace clang::ento::nvm {

class BaseFunction {
protected:
  std::set<const FunctionDecl*> functions;

public:
  bool inFunctions(const FunctionDecl* D) const { return functions.count(D); }

  size_t size() const { return functions.size(); }

  bool empty() const { return functions.empty(); }

  void dump() const {
    for (const FunctionDecl* FD : functions) {
      llvm::errs() << FD->getQualifiedNameAsString() << ",";
    }
    llvm::errs() << "\n";
  }

  void insert(const FunctionDecl* FD) { functions.insert(FD); }
};

} // namespace clang::ento::nvm