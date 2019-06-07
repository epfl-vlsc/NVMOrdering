#pragma once
#include "Common.h"

namespace clang::ento::nvm::ParseUtils {

const MemberExpr* getME(const BinaryOperator* BO) {
  // used for write
  Expr* LHS = BO->getLHS();
  if (const MemberExpr* ME = dyn_cast<MemberExpr>(LHS)) {
    return ME;
  }

  return nullptr;
}

const MemberExpr* getME(const CallExpr* CE) {
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
  if (const MemberExpr* ME = dyn_cast<MemberExpr>(expr)) {
    return ME;
  }

  return nullptr;
}

} // namespace clang::ento::nvm::ParseUtils