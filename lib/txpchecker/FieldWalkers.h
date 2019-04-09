#pragma once
#include "Common.h"
#include "FunctionInfos.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace clang::ento::nvm {

class RangeWalker : public RecursiveASTVisitor<RangeWalker> {
  bool hasMemberExpr;
  ValueDecl* VD;

public:
  RangeWalker() : hasMemberExpr(false), VD(nullptr) {}

  const ValueDecl* getVD() { return (const ValueDecl*)VD; }

  bool VisitMemberExpr(const MemberExpr* ME) {
    if (!hasMemberExpr) {
      VD = ME->getMemberDecl();
      hasMemberExpr = true;
    }

    // continue traversal
    return true;
  }
};

class AssignmentWalker : public RecursiveASTVisitor<AssignmentWalker> {
  bool isObj;
  bool isField;
  ValueDecl* fieldInfo;
  NamedDecl* objInfo;

  const Stmt* getNthChild(Stmt* S, int i) {
    if (!S) {
      return nullptr;
    }

    int c = 0;
    for (Stmt::const_child_iterator I = S->child_begin(), E = S->child_end();
         I != E; ++I) {
      const Stmt* Child = *I;
      if (i == c) {
        return Child;
      }
      c++;
    }

    return nullptr;
  }

  const NamedDecl* getNDFromME(MemberExpr* ME) {
    if (!ME) {
      return nullptr;
    }

    const Stmt* Child1 = getNthChild(ME, 0);
    const Stmt* Child2 = getNthChild((Stmt*)Child1, 0);
    const Stmt* Child3 = getNthChild((Stmt*)Child2, 3);
    const Stmt* Child4 = getNthChild((Stmt*)Child3, 0);
    const Stmt* Child5 = getNthChild((Stmt*)Child4, 1);
    const Stmt* Child6 = getNthChild((Stmt*)Child5, 0);
    const Stmt* Child7 = getNthChild((Stmt*)Child6, 0);
    const Stmt* Child8 = getNthChild((Stmt*)Child7, 0);

    if (const DeclRefExpr* DRE = dyn_cast_or_null<DeclRefExpr>(Child8)) {
      if (const NamedDecl* ND = DRE->getFoundDecl()) {
        return ND;
      }
    }

    return nullptr;
  }

  const DeclRefExpr* getDREFromUO(UnaryOperator* UO) {
    Stmt::const_child_iterator I1 = UO->child_begin();
    const Stmt* Child1 = *I1;
    if (!Child1) {
      return nullptr;
    }
    Stmt::const_child_iterator I2 = Child1->child_begin();
    const Stmt* Child2 = *I2;
    if (!Child2) {
      return nullptr;
    }

    if (const DeclRefExpr* DRE = dyn_cast_or_null<DeclRefExpr>(Child2)) {
      return DRE;
    }

    return nullptr;
  }

public:
  AssignmentWalker() : isObj(false), isField(false) {}

  const ValueDecl* getFieldInfo() { return (const ValueDecl*)fieldInfo; }

  const NamedDecl* getObjInfo() { return (const NamedDecl*)objInfo; }

  bool isObjWrite(){
    return isObj;
  }

  bool isFieldWrite(){
    return isField;
  }

  bool VisitDeclRefExpr(const DeclRefExpr* DRE) {
    if (const NamedDecl* ND = DRE->getFoundDecl()) {
      if (ND->getName().equals("pmemobj_tx_zalloc")) {
        // obj assignment
        isObj = true;
      }
    }

    // continue traversal
    return true;
  }

  bool VisitBinaryOperator(const BinaryOperator* BO) {
    if (BO->isAssignmentOp()) {
      Expr* LHS = BO->getLHS();
      if (MemberExpr* ME = dyn_cast_or_null<MemberExpr>(LHS)) {
        // possible field write

        // field name
        const ValueDecl* VD = ME->getMemberDecl();
        if (VD && !VD->getName().equals("_type")) {
          // field name
          fieldInfo = (ValueDecl*)VD;
        }

        // obj name
        const NamedDecl* ND = getNDFromME(ME);
        if (ND) {
          objInfo = (NamedDecl*)ND;
          isField = true;
        }
      } else if (UnaryOperator* UO = dyn_cast_or_null<UnaryOperator>(LHS)) {
        const DeclRefExpr* DRE = getDREFromUO(UO);
        if (!DRE) {
          return true;
        }

        if (const NamedDecl* ND = DRE->getFoundDecl()) {
          objInfo = (NamedDecl*)ND;
        }
      }
    }

    // continue traversal
    return true;
  }
};

} // namespace clang::ento::nvm