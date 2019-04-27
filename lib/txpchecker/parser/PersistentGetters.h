#pragma once
#include "Common.h"

namespace clang::ento::nvm {

const NamedDecl* getObjFromME(const MemberExpr* ME) {
  if (!ME) {
    return nullptr;
  }

  const Stmt* Child1 = getNthChild(ME, 0);
  const Stmt* Child2 = getNthChild(Child1, 0);
  const Stmt* Child3 = getNthChild(Child2, 3);
  const Stmt* Child4 = getNthChild(Child3, 0);
  const Stmt* Child5 = getNthChild(Child4, 1);
  const Stmt* Child6 = getNthChild(Child5, 0);
  const Stmt* Child7 = getNthChild(Child6, 0);
  const Stmt* Child8 = getNthChild(Child7, 0);

  if (const DeclRefExpr* DRE = dyn_cast_or_null<DeclRefExpr>(Child8)) {
    if (const NamedDecl* ND = DRE->getFoundDecl()) {
      return ND;
    }
  }

  return nullptr;
}

const VarDecl* getVDFromME(const MemberExpr* ME) {
  if (!ME) {
    return nullptr;
  }

  const Stmt* Child1 = getNthChild(ME, 0);
  const Stmt* Child2 = getNthChild(Child1, 0);
  const Stmt* Child3 = getNthChild(Child2, 0);

  if (const DeclStmt* DS = dyn_cast_or_null<DeclStmt>(Child3)) {
    const Decl* BD = DS->getSingleDecl();
    if(const VarDecl* VD = dyn_cast_or_null<VarDecl>(BD)){
      return VD;
    }
  }

  return nullptr;
}

const NamedDecl* getNDFromPE(const ParenExpr* PE) {
  if (!PE) {
    return nullptr;
  }

  const Stmt* Child1 = getNthChild(PE, 0);
  const Stmt* Child2 = getNthChild(Child1, 0);
  const Stmt* Child3 = getNthChild(Child2, 0);
  const Stmt* Child4 = getNthChild(Child3, 0);

  if (const DeclRefExpr* DRE = dyn_cast_or_null<DeclRefExpr>(Child4)) {
    if (const NamedDecl* ND = DRE->getFoundDecl()) {
      return ND;
    }
  }

  return nullptr;
}

const DeclRefExpr* getObjFromUO(const UnaryOperator* UO) {
  if (!UO) {
    return nullptr;
  }

  const Stmt* Child1 = getNthChild(UO, 0);
  const Stmt* Child2 = getNthChild(Child1, 0);

  if (const DeclRefExpr* DRE = dyn_cast_or_null<DeclRefExpr>(Child2)) {
    return DRE;
  }

  return nullptr;
}

} // namespace clang::ento::nvm