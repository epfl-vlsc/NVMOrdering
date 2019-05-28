#pragma once
#include "Common.h"

namespace clang::ento::nvm {

class NameWalker : public ConstStmtVisitor<NameWalker> {
  std::vector<const char*>& names;
  bool usesName;

  void checkName(const StringRef& currentName) {
    for (auto& name : names) {
      if (currentName.equals(name)) {
        DBG("match " << name)
        usesName = true;
      }
    }
  }

public:
  NameWalker(std::vector<const char*>& names_)
      : names(names_), usesName(false) {}

  void VisitStmt(const Stmt* S) { VisitChildren(S); }

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

    if (idInfo) {
      StringRef currentName = idInfo->getName();
      checkName(currentName);
    }

    VisitChildren(DRE);
  }

  void VisitMemberExpr(const MemberExpr* ME) {
    const ValueDecl* VD = ME->getMemberDecl();
    StringRef currentName = VD->getName();
    checkName(currentName);

    VisitChildren(ME);
  }

  bool isNameUsed() { return usesName; }
}; // namespace clang::ento::nvm

bool usesNames(const Stmt* S, std::vector<const char*>& names) {
  NameWalker nameWalker(names);
  nameWalker.Visit(S);
  return nameWalker.isNameUsed();
}

} // namespace clang::ento::nvm