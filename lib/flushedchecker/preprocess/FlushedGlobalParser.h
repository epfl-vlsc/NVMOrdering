#pragma once
#include "Common.h"
#include "parser_util/ParseUtils.h"

namespace clang::ento::nvm {

template <typename Globals>
class FlushedGlobalParser : public RecursiveASTVisitor<FlushedGlobalParser<Globals>> {
public:
  using LatVar = typename Globals::LatVar;
  using BOset = std::set<const BinaryOperator*>;

  // whole program data structures
  Globals& globals;
  ASTContext& AC;

  // temporary structures
  BOset binaryOperators;

  void autoFindFunctionsVariables() {
    for (auto BO : binaryOperators) {
      auto fieldME = ParseUtils::getFieldDeclFromWriteME(BO);
      if (!fieldME) {
        continue;
      }

      auto field = fieldME->getMemberDecl();
      if (field && !globals.isFieldVariable(field)) {
        continue;
      }

      auto rhs = ParseUtils::getNDOfRHS(BO);
      if (rhs) {
        globals.insertTrackedVariable(rhs);
      }

      const FunctionDecl* FD = getFuncDecl(fieldME, AC);
      if (!FD) {
        continue;
      }

      // add function to the analysis list
      globals.insertAnalyzeFunction(FD);
    }

    // remove all skip from analyze
    globals.removeSkipFromAnalyze();
  }

  void trackStmt(const FunctionDecl* FD) {
    if (FD->hasBody() && !globals.isSkipFunction(FD)) {
      const Stmt* functionBody = FD->getBody();
      trackStmt(functionBody);
    }
  }

  void trackStmt(const Stmt* S) {
    if (auto CE = dyn_cast<CallExpr>(S)) {
      const FunctionDecl* FD = CE->getDirectCallee();
      if (FD->hasBody() && !globals.isSkipFunction(FD)) {
        trackStmt(FD);
        globals.insertFunctionToActiveUnit(FD);
      } else {
        return;
      }
    } else if (auto BO = dyn_cast<BinaryOperator>(S)) {
      auto field = ParseUtils::getFieldDeclFromWrite(BO);
      if (!field) {
        trackChildren(S);
        return;
      }

      if (!globals.isFieldVariable(field)) {
        trackChildren(S);
        return;
      }

      auto rhs = ParseUtils::getNDOfRHS(BO);
      if (!rhs) {
        trackChildren(S);
        return;
      }

      if (globals.isTrackedVariable(rhs)) {
        globals.insertVariableToActiveUnit(rhs);
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
    }
  }

public:
  FlushedGlobalParser(TranslationUnitDecl* TUD, Globals& globals, ASTContext& AC_)
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
    globals.insertFieldVariable(FD);

    // continue traversal
    return true;
  }

  bool VisitRecordDecl(const RecordDecl* RD) {

    // continue traversal
    return true;
  }

  bool VisitBinaryOperator(const BinaryOperator* BO) {
    if (BO->isAssignmentOp()) {
      binaryOperators.insert(BO);
    }

    // continue traversal
    return true;
  }

  bool shouldVisitTemplateInstantiations() const { return true; }

  void fillStructures() {
    autoFindFunctionsVariables();
    fillTrackMap();
  }
};

} // namespace clang::ento::nvm