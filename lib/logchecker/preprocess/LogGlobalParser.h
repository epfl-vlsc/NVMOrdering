#pragma once
#include "Common.h"

namespace clang::ento::nvm {

template <typename Globals>
class LogGlobalParser : public RecursiveASTVisitor<LogGlobalParser<Globals>> {
public:
  using LatVar = typename Globals::LatVar;

  // whole program data structures
  Globals& globals;
  ASTContext& AC;

  void trackStmt(const FunctionDecl* FD) {
    if (FD->hasBody() && !globals.isSkipFunction(FD)) {
      const Stmt* functionBody = FD->getBody();
      trackStmt(functionBody);
    }
  }

  void trackStmt(const Stmt* S) {
    if (const CallExpr* CE = dyn_cast<CallExpr>(S)) {
      const FunctionDecl* FD = CE->getDirectCallee();
      if (FD->hasBody() && !globals.isSkipFunction(FD)) {
        trackStmt(FD);
        globals.insertFunctionToActiveUnit(FD);
      } else {
        return;
      }
    } else if (const MemberExpr* ME = dyn_cast<MemberExpr>(S)) {

      const ValueDecl* VD = ME->getMemberDecl();
      if (globals.isUsedVar(VD)) {
        globals.insertVariableToActiveUnit(VD);
      }
    }

    trackChildren(S);
  }

  void trackChildren(const Stmt* S) {
    for (Stmt::const_child_iterator I = S->child_begin(), E = S->child_end();
         I != E; ++I) {
      if (const Stmt* Child = *I) {
        trackStmt(Child);
      }
    }
  }

  void fillTrackMap() {
    for (const FunctionDecl* FD : globals.getAnalyzedFunctions()) {
      globals.initActiveUnit(FD);
      trackStmt(FD);

      //add tx variable
      auto tx = globals.getTxFunction();
      globals.insertVariableToActiveUnit(tx);
    }
  }

public:
  LogGlobalParser(TranslationUnitDecl* TUD, Globals& globals, ASTContext& AC_)
      : globals(globals), AC(AC_) {
    this->TraverseDecl(TUD);
    this->fillStructures();
  }

  bool VisitFunctionDecl(const FunctionDecl* FD) {
    globals.insertFunction(FD);

    // continue traversal
    return true;
  }

  bool VisitFieldDecl(const FieldDecl* FD) {
    globals.insertVariable(FD);

    // continue traversal
    return true;
  }

  bool VisitRecordDecl(const RecordDecl* RD) {
    globals.insertVariable(RD);

    // continue traversal
    return true;
  }

  bool shouldVisitTemplateInstantiations() const { return true; }

  void fillStructures() {
    globals.setTxMode();
    fillTrackMap();
  }
};

} // namespace clang::ento::nvm