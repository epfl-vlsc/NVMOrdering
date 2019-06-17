#pragma once
#include "Common.h"
#include "identify/AnnotVar.h"
#include "identify/MainFncs.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace clang::ento::nvm {

using PtrFncs = MainFncs;
using PtrVars = AnnotVar;

class PtrWalker : public RecursiveASTVisitor<PtrWalker> {
  using MESet = std::set<const MemberExpr*>;

  PtrFncs& ptrFncs;
  PtrVars& ptrVars;
  ASTContext& AC;

  MESet memberExprs;

  void fillAnalysisFunctions() {
    for (const MemberExpr* ME : memberExprs) {
      const ValueDecl* VD = ME->getMemberDecl();
      if (!ptrVars.isUsedVar(VD)) {
        continue;
      }

      const FunctionDecl* FD = getFuncDecl(ME, AC);
      if (!FD) {
        continue;
      }

      ptrFncs.insertAnalyze(FD);
    }
  }

public:
  PtrWalker(PtrFncs& ptrFncs_, PtrVars& ptrVars_, ASTContext& AC_)
      : ptrFncs(ptrFncs_), ptrVars(ptrVars_), AC(AC_) {}

  bool VisitFunctionDecl(const FunctionDecl* FD) {
    ptrFncs.insertIfKnown(FD);

    // continue traversal
    return true;
  }

  bool VisitFieldDecl(const FieldDecl* FD) {
    ptrVars.insertIfKnown(FD);

    // continue traversal
    return true;
  }

  bool VisitMemberExpr(const MemberExpr* ME) {
    memberExprs.insert(ME);

    // continue traversal
    return true;
  }

  bool shouldVisitTemplateInstantiations() const { return true; }

  void finalize() {
    fillAnalysisFunctions();
  }

  // bool shouldVisitImplicitCode() const { return true; }
};

} // namespace clang::ento::nvm