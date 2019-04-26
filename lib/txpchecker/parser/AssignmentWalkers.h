#pragma once
#include "Common.h"
#include "FncNames.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace clang::ento::nvm {

// todo handle write to obj

class AssignmentWalker : public RecursiveASTVisitor<AssignmentWalker> {
  enum K { W_OBJ, W_FIELD, I_OBJ, NONE } Kind;
  NamedDecl* objND;
  NamedDecl* fieldND;

  const NamedDecl* getObjFromME(const MemberExpr* ME) const {
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

  const DeclRefExpr* getObjFromUO(const UnaryOperator* UO) const {
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

  void setFieldME(const MemberExpr* ME) {
    const ValueDecl* VD = ME->getMemberDecl();
    if (VD && !VD->getName().equals("_type")) {
      // field name
      fieldND = (NamedDecl*)VD;
    }
  }

  void setObjME(const MemberExpr* ME) {
    const NamedDecl* ND = getObjFromME(ME);
    objND = (NamedDecl*)ND;
  }

  void setObjUO(const UnaryOperator* UO) {
    const DeclRefExpr* DRE = getObjFromUO(UO);

    if (DRE) {
      const NamedDecl* ND = DRE->getFoundDecl();
      objND = (NamedDecl*)ND;
    }
  }

  void extractNDs(const Expr* E) {
    if (const MemberExpr* ME = dyn_cast_or_null<MemberExpr>(E)) {
      // possible field write
      // field
      setFieldME(ME);

      // obj
      setObjME(ME);

      if (objND && fieldND) {
        Kind = K::W_FIELD;
      }
    } else if (const UnaryOperator* UO = dyn_cast_or_null<UnaryOperator>(E)) {
      // extract obj
      setObjUO(UO);

      if(objND){
        Kind = K::W_OBJ;
      }
    }
  }

public:
  AssignmentWalker() : Kind(K::NONE), objND(nullptr), fieldND(nullptr) {}

  const NamedDecl* getObjND() { return (const NamedDecl*)objND; }

  const NamedDecl* getFieldND() { return (const NamedDecl*)fieldND; }

  bool isWriteObj() { return Kind == K::W_OBJ; }

  bool isWriteField() { return Kind == K::W_FIELD; }

  bool isInitObj() { return Kind == K::I_OBJ; }

  bool isUsed() { return Kind != K::NONE; }

  // for capturing alloc
  bool VisitDeclRefExpr(const DeclRefExpr* DRE) {
    if (const NamedDecl* ND = DRE->getFoundDecl()) {
      const IdentifierInfo* II = ND->getIdentifier();

      if (II && inAlloc(II)) {
        // obj assignment
        Kind = K::I_OBJ;
        return false;
      }
    }

    // continue traversal
    return true;
  }

  bool VisitBinaryOperator(const BinaryOperator* BO) {
    if (BO->isAssignmentOp()) {
      Expr* LHS = BO->getLHS();

      // for capturing obj and fields
      extractNDs(LHS);
    }

    // continue traversal
    return true;
  }
};

} // namespace clang::ento::nvm