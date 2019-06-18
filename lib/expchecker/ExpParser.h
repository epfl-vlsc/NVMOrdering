#pragma once
#include "Common.h"
#include "parser_util/ParseUtils.h"

namespace clang::ento::nvm {

class ExpParser : public RecursiveASTVisitor<ExpParser> {
public:
  ExpParser(TranslationUnitDecl* TUD) { this->TraverseDecl(TUD); }

  bool VisitCallExpr(const CallExpr* CE) {
    printStmt(CE, "ce");
    
    auto ND = ParseUtils::getPtrFromFlush(CE);

    if(ND)
    printND(ND, "cend");


    auto ND2 = ParseUtils::getFieldDeclFromCall(CE);

    if(ND2)
    printND(ND2, "cend2");


    // continue traversal
    return true;
  }

  bool VisitBinaryOperator(const BinaryOperator* BO) {
    printStmt(BO, "bo");

    auto ND = ParseUtils::getNDOfRHS(BO);

    if(ND)
    printND(ND, "nd");

    auto ND2 = ParseUtils::getNDOfAssigned(BO);

    if(ND2)
    printND(ND2, "nd2");

    auto ND3 = ParseUtils::getFieldDeclFromWrite(BO);

    if(ND3)
    printND(ND3, "nd3");

    // continue traversal
    return true;
  }

  bool VisitFunctionDecl(const FunctionDecl* FD) {
    // printND(FD, "fd");

    // continue traversal
    return true;
  }

  bool VisitFieldDecl(const FieldDecl* FD) {
    // printND(FD, "fd");

    // continue traversal
    return true;
  }

  bool VisitRecordDecl(const RecordDecl* RD) {
    // printND(RD, "rd");

    // continue traversal
    return true;
  }

  bool shouldVisitTemplateInstantiations() const { return true; }
};

} // namespace clang::ento::nvm