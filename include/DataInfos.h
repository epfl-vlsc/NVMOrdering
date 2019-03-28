#pragma once

#include "AstWalkers.h"
#include "Common.h"

namespace clang::ento::nvm {

class VarInfos {
  using BIVec = std::vector<BaseInfo*>;
  using ValueMap = std::map<const ValueDecl*, BIVec>;
  ValueMap usedVars;

public:
  void collectUsedVars(const TranslationUnitDecl* CTUD) {
    TranslationUnitDecl* TUD = (TranslationUnitDecl*)CTUD;
    VarWalker varWalker(usedVars);
    varWalker.TraverseDecl(TUD);
    varWalker.createUsedVars();
  }

  void dump() {
    for (auto& [VD, BIV] : usedVars) {
      llvm::outs() << VD->getQualifiedNameAsString() << "\n";
      for (auto& BI : BIV) {
        llvm::outs() << "\t";
        BI->dump();
        llvm::outs() << "\n";
      }
    }
  }
};

} // namespace clang::ento::nvm