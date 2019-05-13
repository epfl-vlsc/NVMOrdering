#pragma once
#include "Common.h"

namespace clang::ento::nvm {

class NameWalker : public ConstStmtVisitor<NameWalker> {
  StringRef name;
  bool usesName;

public:
  NameWalker(StringRef name_) : name(name_), usesName(false) {}

  void VisitStmt(const Stmt* S) {
    VisitChildren(S);
  }

  void VisitChildren(const Stmt* S) {
    for (Stmt::const_child_iterator I = S->child_begin(), E = S->child_end();
         I != E; ++I) {
      if (const Stmt* Child = *I) {
        Visit(Child);
      }
    }
  }

  void VisitDeclRefExpr(const DeclRefExpr* DRE) {

    auto* idInfo = DRE->getNameInfo().getName().getAsIdentifierInfo();
    
    if(idInfo){
      StringRef currentName = idInfo->getName();
    // llvm::outs() << currentMask << " " << mask << "\n";
      if (currentName.equals(name)) {
        usesName = true;
      }
    }

    VisitChildren(DRE);
  }

  bool isNameUsed() { return usesName; }
};

bool usesName(const Stmt* S, StringRef name) {
  NameWalker nameWalker(name);
  nameWalker.Visit(S);
  return nameWalker.isNameUsed();
}


} // namespace clang::ento::nvm