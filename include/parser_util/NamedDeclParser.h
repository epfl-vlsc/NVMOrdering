#pragma once
#include "Common.h"

namespace clang::ento::nvm {

class NamedDeclParser {
  using VarSet = std::set<const NamedDecl*>;
  using FuncSet = std::set<const FunctionDecl*>;

  VarSet varSet;
  FuncSet funcSet;

  void VisitStmt(const FunctionDecl* FD) {
    if (FD->hasBody() && !funcSet.count(FD)) {
      const Stmt* functionBody = FD->getBody();
      VisitStmt(functionBody);
    }
  }

  void VisitStmt(const Stmt* S) {
    if (const CallExpr* CE = dyn_cast<CallExpr>(S)) {
      const FunctionDecl* FD = CE->getDirectCallee();
      if (FD->hasBody() && !funcSet.count(FD)) {
        const Stmt* functionBody = FD->getBody();
        VisitStmt(functionBody);
      }
      funcSet.insert(FD);
    } else if (const MemberExpr* ME = dyn_cast<MemberExpr>(S)) {
      const ValueDecl* VD = ME->getMemberDecl();
      varSet.insert(VD);
    }

    VisitChildren(S);
  }

  void VisitChildren(const Stmt* S) {
    for (Stmt::const_child_iterator I = S->child_begin(), E = S->child_end();
         I != E; ++I) {
      if (const Stmt* Child = *I) {
        VisitStmt(Child);
      }
    }
  }

public:
  NamedDeclParser(const FunctionDecl* FD) { VisitStmt(FD); }

  VarSet& getVarSet() { return varSet; }

  FuncSet& getFuncSet() { return funcSet; }
};

} // namespace clang::ento::nvm