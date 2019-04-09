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

  bool hasME() { return hasMemberExpr; }
};

class AssignmentWalker : public RecursiveASTVisitor<AssignmentWalker> {
  bool isObj;
  bool isField;

public:
  AssignmentWalker() : isObj(false), isField(false) {}

  const Stmt* getNthChild(Stmt* S, int i) {
    if (!S) {
      return nullptr;
    }

    int c = 0;
    for (Stmt::const_child_iterator I = S->child_begin(), E = S->child_end();
         I != E; ++I) {
      const Stmt* Child = *I;
      if(i == c){
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

  bool VisitDeclRefExpr(const DeclRefExpr* DRE) {
    if (const NamedDecl* ND = DRE->getFoundDecl()) {
      // tx_zalloc
      llvm::errs() << "function: " << ND->getNameAsString() << "\n";
    }
    llvm::errs() << "\n";

    // continue traversal
    return true;
  }

  bool VisitBinaryOperator(const BinaryOperator* BO) {
    llvm::errs() << "BO\n";
    if (BO->isAssignmentOp()) {
      Expr* LHS = BO->getLHS();
      if (MemberExpr* ME = dyn_cast_or_null<MemberExpr>(LHS)) {
        llvm::errs() << "ME\n";
        // field name
        const ValueDecl* VD = ME->getMemberDecl();

        if (VD && !VD->getName().equals("_type")) {
          // field name
          llvm::errs() << "field: " << VD->getNameAsString() << "\n";
        }

        const NamedDecl* ND = getNDFromME(ME);
        if (ND) {
          // obj name
          llvm::errs() << "obj: " << ND->getNameAsString() << "\n";
        }

        int i = 0;
      } else if (UnaryOperator* UO = dyn_cast_or_null<UnaryOperator>(LHS)) {
        const DeclRefExpr* DRE = getDREFromUO(UO);
        if (!DRE) {
          return true;
        }

        if (const NamedDecl* ND = DRE->getFoundDecl()) {
          // obj assignment
          llvm::errs() << "obj assignment: " << ND->getNameAsString() << "\n";
        }
      }
    }

    // continue traversal
    return true;
  }
};

} // namespace clang::ento::nvm