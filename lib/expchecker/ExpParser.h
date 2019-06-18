#pragma once
#include "Common.h"

namespace clang::ento::nvm {

class ExpParser : public RecursiveASTVisitor<ExpParser> {
public:
  ExpParser(TranslationUnitDecl* TUD) { this->TraverseDecl(TUD); }

  bool VisitCallExpr(const CallExpr* CE) {
    printStmt(CE, "ce");

    // continue traversal
    return true;
  }

  bool VisitBinaryOperator(const BinaryOperator* BO) {
    printStmt(BO, "bo");

    // continue traversal
    return true;
  }

  bool VisitFunctionDecl(const FunctionDecl* FD) {
    printND(FD, "fd");

    // continue traversal
    return true;
  }

  bool VisitFieldDecl(const FieldDecl* FD) {
    printND(FD, "fd");

    // continue traversal
    return true;
  }

  bool VisitRecordDecl(const RecordDecl* RD) {
    printND(RD, "rd");

    // continue traversal
    return true;
  }

  bool shouldVisitTemplateInstantiations() const { return true; }
};

} // namespace clang::ento::nvm