#pragma once
#include "Common.h"

namespace clang::ento::nvm {

class NDWalker {
  std::set<const NamedDecl*> vars;
  std::set<const FunctionDecl*> seenFunction;

  void VisitStmt(const FunctionDecl* FD) {
    if (FD->hasBody() && !seenFunction.count(FD)) {
      const Stmt* functionBody = FD->getBody();
      VisitStmt(functionBody);
    }
  }

  void VisitStmt(const Stmt* S) {
    if (const CallExpr* CE = dyn_cast<CallExpr>(S)) {
      const FunctionDecl* FD = CE->getDirectCallee();
      if (FD->hasBody() && !seenFunction.count(FD)) {
        const Stmt* functionBody = FD->getBody();
        VisitStmt(functionBody);
      }

      seenFunction.insert(FD);
    } else if (const MemberExpr* ME = dyn_cast<MemberExpr>(S)) {
      const ValueDecl* VD = ME->getMemberDecl();
      vars.insert(VD);
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
  auto begin() const { return vars.cbegin(); }

  auto end() const { return vars.cend(); }

  static NDWalker getNDs(const FunctionDecl* FD) {
    NDWalker ndw;
    ndw.VisitStmt(FD);
    return ndw;
  }
}; // namespace clang::ento::nvm

} // namespace clang::ento::nvm