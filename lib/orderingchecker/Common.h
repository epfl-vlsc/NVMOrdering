#pragma once
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerRegistry.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/AST/StmtVisitor.h"
#include "llvm/Support/raw_ostream.h"
#include <utility>

namespace clang::ento::nvm {

const FunctionDecl* getFuncDecl(const Decl* BD);

const FunctionDecl* getFuncDecl(const CallEvent& Call);

const FunctionDecl* getFuncDecl(const LocationContext* LC);

const DeclaratorDecl* getDeclaratorDecl(const Decl* BD);

bool isTopFunction(CheckerContext& C);

class MaskWalker : public ConstStmtVisitor<MaskWalker> {
  bool usesMask_;
  StringRef mask_;

public:
  MaskWalker(StringRef mask);

  void VisitDeclRefExpr(const DeclRefExpr* DRE);

  void VisitStmt(const Stmt* S);

  void VisitUnaryOperator(const UnaryOperator* UOp);

  void VisitChildren(const Stmt* S);

  bool usesMask();
};

bool usesMask(const Stmt* S, StringRef mask);

} // namespace clang::ento::nvm