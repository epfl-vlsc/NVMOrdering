#pragma once
#include "Common.h"
#include "identify/AnnotVar.h"
#include "identify/MainFncs.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace clang::ento::nvm {

using PtrFncs = MainFncs;
using PtrVars = AnnotVar;

class PtrWalker : public RecursiveASTVisitor<PtrWalker> {
  PtrFncs& ptrFncs;
  PtrVars& ptrVars;

public:
  PtrWalker(PtrFncs& ptrFncs_, PtrVars& ptrVars_)
      : ptrFncs(ptrFncs_), ptrVars(ptrVars_) {}

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

  bool shouldVisitTemplateInstantiations() const { return true; }

  //bool shouldVisitImplicitCode() const { return true; }
};

} // namespace clang::ento::nvm