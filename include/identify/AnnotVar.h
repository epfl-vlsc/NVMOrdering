#pragma once

#include "Common.h"
#include <set>

namespace clang::ento::nvm {

class AnnotVar {
  using ValueSet = std::set<const NamedDecl*>;
  ValueSet valueSet;
  const char* specialAnnot;

public:
  AnnotVar(const char* specialAnnot_) : specialAnnot(specialAnnot_) {}
  virtual ~AnnotVar() {}

  void addBasedOnAnnot(const NamedDecl* ND, const AnnotateAttr* AA) {
    StringRef annotation = AA->getAnnotation();
    if (annotation.contains(specialAnnot)) {
      valueSet.insert(ND);
    }
  }

  void insertIfKnown(const NamedDecl* ND) {
    for (const auto* AA : ND->specific_attrs<AnnotateAttr>()) {
      // add to annotated vars
      addBasedOnAnnot(ND, AA);
    }
  }

  bool inValues(const NamedDecl* ND) const { return valueSet.count(ND); }

  void dump() {
    for (const NamedDecl* ND : valueSet) {
      llvm::outs() << ND->getQualifiedNameAsString() << "\n";
    }
  }
};

} // namespace clang::ento::nvm