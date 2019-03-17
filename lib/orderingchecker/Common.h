#pragma once
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerRegistry.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/AST/StmtVisitor.h"
#include "llvm/Support/raw_ostream.h"
#include <utility>

namespace clang::ento::nvm {

const FunctionDecl* getFuncDecl(const Decl* BD) {
  if (const FunctionDecl* D = dyn_cast_or_null<FunctionDecl>(BD)) {
    return D;
  }
  llvm::report_fatal_error("use only for functions");
  return nullptr;
}

const FunctionDecl* getFuncDecl(const CallEvent& Call) {
  const Decl* BD = Call.getDecl();
  return getFuncDecl(BD);
}

const FunctionDecl* getFuncDecl(const LocationContext* LC) {
  const Decl* BD = LC->getDecl();
  return getFuncDecl(BD);
}

const DeclaratorDecl* getDeclaratorDecl(const Decl* BD) {
  if (const DeclaratorDecl* D = dyn_cast_or_null<DeclaratorDecl>(BD)) {
    return D;
  }
  llvm::report_fatal_error("All calls have function declaration");
  return nullptr;
}

bool isTopFunction(CheckerContext& C) { return C.inTopFrame(); }

class MaskWalker : public ConstStmtVisitor<MaskWalker> {
  bool usesMask_;
  StringRef mask_;

public:
  void VisitDeclRefExpr(const DeclRefExpr* DRE) {
  StringRef currentMask =
      DRE->getNameInfo().getName().getAsIdentifierInfo()->getName();
  if (currentMask.equals(mask_)) {
    usesMask_ = true;
  }
}

MaskWalker(StringRef mask) : usesMask_(false), mask_(mask) {}

void VisitStmt(const Stmt* S) { VisitChildren(S); }

void VisitUnaryOperator(const UnaryOperator* UOp) {
  // using another mask
  usesMask_ = false;
}

void VisitChildren(const Stmt* S) {
  for (Stmt::const_child_iterator I = S->child_begin(), E = S->child_end();
       I != E; ++I) {
    if (const Stmt* Child = *I) {
      Visit(Child);
    }
  }
}

bool usesMask() { return usesMask_; }
};

bool usesMask(const Stmt* S, StringRef mask) {
  MaskWalker maskWalker(mask);
  maskWalker.Visit(S);
  return maskWalker.usesMask();
}

} // namespace clang::ento::nvm