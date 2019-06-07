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

  void insertIfKnown(const FunctionDecl* D) {
    // if has attribute for analysis, add function to set
    for (const auto* Ann : D->specific_attrs<AnnotateAttr>()) {
      if (checkName(Ann)) {
        functions.insert(D);
      }
    }
  }

  void erase(const FunctionDecl* FD) { functions.erase(FD); }

  auto begin() const { return functions.cbegin(); }

  auto end() const { return functions.cend(); }

  void dump() const{
    llvm::errs() << annotation << " ";
    BaseFunction::dump();
  }
};

} // namespace clang::ento::nvm