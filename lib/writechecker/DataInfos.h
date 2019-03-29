#pragma once

#include "AstWalkers.h"
#include "Common.h"

namespace clang::ento::nvm {

class VarInfos {
  using InfoList = std::vector<BaseInfo*>;
  using ValueMap = std::map<const ValueDecl*, InfoList>;
  ValueMap usedVars;

public:
  void collectUsedVars(const TranslationUnitDecl* CTUD) {
    TranslationUnitDecl* TUD = (TranslationUnitDecl*)CTUD;
    VarWalker varWalker(usedVars);
    varWalker.TraverseDecl(TUD);
    varWalker.createUsedVars();
  }

  InfoList& getInfoList(const ValueDecl* VD){
    assert(usedVars.count(VD));
    return usedVars[VD];
  }

  bool isUsedVar(const ValueDecl* VD){
    return usedVars.count(VD);
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