#pragma once

#include "Common.h"
#include <set>

namespace clang::ento::nvm {

class SpecialValue {
  using ValueSet = std::set<const ValueDecl*>;
  ValueSet valueSet;
  const char* specialAnnot;

public:
  SpecialValue(const char* specialAnnot_) : specialAnnot(specialAnnot_) {}
  virtual ~SpecialValue() {}

  void addBasedOnAnnot(const ValueDecl* VD, const AnnotateAttr* AA) {
    StringRef annotation = AA->getAnnotation();
    if (annotation.contains(specialAnnot)) {
      valueSet.insert(VD);
    }
  }

  void insertIfKnown(const ValueDecl* VD) {
    for (const auto* AA : VD->specific_attrs<AnnotateAttr>()) {
      // add to annotated vars
      addBasedOnAnnot(VD, AA);
    }
  }

  bool inValues(const ValueDecl* VD) const { return valueSet.count(VD); }

  void dump() {
    for (const ValueDecl* VD : valueSet) {
      llvm::outs() << VD->getQualifiedNameAsString() << "\n";
    }
  }
};

} // namespace clang::ento::nvm