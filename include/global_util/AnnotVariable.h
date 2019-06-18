#pragma once

#include "Common.h"
#include <set>

namespace clang::ento::nvm {

class AnnotVariable {
  using VariableSet = std::set<const NamedDecl*>;
  VariableSet variableSet;
  const char* specialAnnot;

public:
  AnnotVariable(const char* specialAnnot_) : specialAnnot(specialAnnot_) {}
  virtual ~AnnotVariable() {}

  void insertVariable(const NamedDecl* ND, const AnnotateAttr* AA) {
    StringRef annotation = AA->getAnnotation();

    if (annotation.contains(specialAnnot)) {
      variableSet.insert(ND);
    }
  }

  void insertVariable(const NamedDecl* ND) {
    for (const auto* AA : ND->specific_attrs<AnnotateAttr>()) {
      // add to annotated vars
      insertVariable(ND, AA);
    }
  }

  bool isUsedVar(const NamedDecl* ND) const { return variableSet.count(ND); }

  void dump() const {
    for (const NamedDecl* ND : variableSet) {
      llvm::errs() << ND->getQualifiedNameAsString() << "\n";
    }
  }

  auto getSpecialAnnotation() const { return specialAnnot; }

  auto count(const NamedDecl* ND) const { return variableSet.count(ND); }

  auto begin() const { return variableSet.begin(); }

  auto end() const { return variableSet.end(); }
};

} // namespace clang::ento::nvm