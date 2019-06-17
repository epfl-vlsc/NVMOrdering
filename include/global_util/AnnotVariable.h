#pragma once

#include "Common.h"
#include <set>

namespace clang::ento::nvm {

class AnnotVariable {
  using ValueSet = std::set<const NamedDecl*>;
  ValueSet valueSet;
  const char* specialAnnot;

public:
  AnnotVariable(const char* specialAnnot_) : specialAnnot(specialAnnot_) {}
  virtual ~AnnotVariable() {}

  void insertVariable(const NamedDecl* ND, const AnnotateAttr* AA) {
    StringRef annotation = AA->getAnnotation();

    if (annotation.contains(specialAnnot)) {
      valueSet.insert(ND);
    }
  }

  void insertVariable(const NamedDecl* ND) {
    for (const auto* AA : ND->specific_attrs<AnnotateAttr>()) {
      // add to annotated vars
      insertVariable(ND, AA);
    }
  }

  bool isUsedVar(const NamedDecl* ND) const { return valueSet.count(ND); }

  void dump() const {
    for (const NamedDecl* ND : valueSet) {
      llvm::errs() << ND->getQualifiedNameAsString() << "\n";
    }
  }

  const char* getSpecialAnnotation() const { return specialAnnot; }
};

} // namespace clang::ento::nvm