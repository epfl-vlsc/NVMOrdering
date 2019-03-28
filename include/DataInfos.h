#pragma once

#include "AstWalkers.h"
#include "Common.h"

namespace clang::ento::nvm {

class VarInfos {
  using DeclInfoMap = std::map<const ValueDecl*, BaseInfo*>;
  DeclInfoMap annotVars;

public:
  void collectUsedVars(const TranslationUnitDecl* CTUD) {
    TranslationUnitDecl* TUD = (TranslationUnitDecl*)CTUD;
    VarWalker varWalker(annotVars);
    varWalker.TraverseDecl(TUD);
    varWalker.createAnnotVars();
  }


  void dump(){
    for(auto& [VD, BI]: annotVars){
      llvm::outs() << VD->getQualifiedNameAsString() << "\n";
    }
  }
};

} // namespace clang::ento::nvm