#pragma once
#include "Common.h"

namespace clang::ento::nvm::ParseUtils {

const NamedDecl* getNDOfRHS(const BinaryOperator* BO) {
  if (!BO->isAssignmentOp()) {
    return nullptr;
  }

  // used for write
  Expr* RHS = BO->getRHS();
  const ImplicitCastExpr* ICE = dyn_cast_or_null<ImplicitCastExpr>(RHS);
  if (!ICE) {
    return nullptr;
  }

  const Stmt* Child1 = getNthChild(ICE, 0);
  const DeclRefExpr* DRE = dyn_cast_or_null<DeclRefExpr>(Child1);
  if (!DRE) {
    return nullptr;
  }

  return DRE->getFoundDecl();
}

const NamedDecl* getNDOfAssigned(const BinaryOperator* BO) {
  if (!BO->isAssignmentOp()) {
    return nullptr;
  }

  // used for write
  Expr* LHS = BO->getLHS();
  const DeclRefExpr* DRE = dyn_cast_or_null<DeclRefExpr>(LHS);
  if (!DRE) {
    return nullptr;
  }

  return DRE->getFoundDecl();
}

const NamedDecl* getPtrFromFlush(const CallExpr* CE) {
  // used for flush
  const Expr* arg0 = CE->getArg(0);
  const ImplicitCastExpr* ICE = dyn_cast_or_null<ImplicitCastExpr>(arg0);
  if (!ICE) {
    return nullptr;
  }

  const Stmt* Child1 = getNthChild(ICE, 0);
  const ImplicitCastExpr* ICE2 = dyn_cast_or_null<ImplicitCastExpr>(Child1);
  if (!ICE2) {
    return nullptr;
  }

  const Stmt* Child2 = getNthChild(ICE2, 0);
  const DeclRefExpr* DRE = dyn_cast_or_null<DeclRefExpr>(Child2);
  if (!DRE) {
    return nullptr;
  }

  return DRE->getFoundDecl();
}

const MemberExpr* getFieldDeclFromWriteME(const BinaryOperator* BO) {
  if (!BO->isAssignmentOp()) {
    return nullptr;
  }

  // used for write
  Expr* LHS = BO->getLHS();
  const MemberExpr* ME = dyn_cast_or_null<MemberExpr>(LHS);
  return ME;
}

const ValueDecl* getFieldDeclFromWrite(const BinaryOperator* BO) {
  const MemberExpr* ME = getFieldDeclFromWriteME(BO);
  if (!ME) {
    return nullptr;
  }

  const ValueDecl* VD = ME->getMemberDecl();
  return VD;
}

const ValueDecl* getFieldDeclFromCall(const CallExpr* CE) {
  // used for flush
  const Expr* arg0 = CE->getArg(0);
  if (!isa<ImplicitCastExpr>(arg0)) {
    return nullptr;
  }

  const ImplicitCastExpr* ICE = dyn_cast<ImplicitCastExpr>(arg0);
  const Stmt* Child1 = getNthChild(ICE, 0);
  if (!isa<UnaryOperator>(Child1)) {
    return nullptr;
  }

  const UnaryOperator* UO = dyn_cast<UnaryOperator>(Child1);
  const Stmt* expr = UO->getSubExpr();
  const MemberExpr* ME = dyn_cast_or_null<MemberExpr>(expr);
  if (!ME) {
    return nullptr;
  }

  const ValueDecl* VD = ME->getMemberDecl();
  return VD;
}

} // namespace clang::ento::nvm::ParseUtils