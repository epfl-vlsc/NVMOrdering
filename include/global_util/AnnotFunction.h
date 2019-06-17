#pragma once
#include "BaseFunction.h"

namespace clang::ento::nvm {

class AnnotFunction : public BaseFunction {
  const char* annotation;

public:
  AnnotFunction(const char* annotation_) : annotation(annotation_) {}

  bool checkName(const AnnotateAttr* AA) const {
    return AA && AA->getAnnotation() == annotation;
  }

  void insertFunction(const FunctionDecl* FD) {
    // if has attribute for analysis, add function to set
    for (const auto* Ann : FD->specific_attrs<AnnotateAttr>()) {
      if (checkName(Ann)) {
        functions.insert(FD);
      }
    }
  }

  void insertFunctionDirect(const FunctionDecl* FD) { functions.insert(FD); }

  void erase(const FunctionDecl* FD) { functions.erase(FD); }

  auto begin() const { return functions.cbegin(); }

  auto end() const { return functions.cend(); }

  void dump() const {
    llvm::errs() << annotation << " ";
    BaseFunction::dump();
  }
};

} // namespace clang::ento::nvm