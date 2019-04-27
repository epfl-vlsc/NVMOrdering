#pragma once
#include "Common.h"
#include "FncNames.h"
#include "PersistentGetters.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace clang::ento::nvm {

// todo handle write to obj

class AssignmentWalker {
  enum K { W_OBJ, W_FIELD, I_OBJ, NONE } Kind;
  NamedDecl* objND;
  NamedDecl* fieldND;

  void setFieldME(const MemberExpr* ME) {
    const ValueDecl* VD = ME->getMemberDecl();
    if (VD && !VD->getName().equals("_type")) {
      // field name
      fieldND = (NamedDecl*)VD;
      llvm::errs() << "field:" << fieldND << "\n";
    }
  }

  void setObjME(const MemberExpr* ME) {
    const NamedDecl* ND = getObjFromME(ME);
    objND = (NamedDecl*)ND;
    llvm::errs() << "objme:" << objND << "\n";
  }

  void setObjUO(const UnaryOperator* UO) {
    const DeclRefExpr* DRE = getObjFromUO(UO);

    if (DRE) {
      const NamedDecl* ND = DRE->getFoundDecl();
      objND = (NamedDecl*)ND;
      llvm::errs() << "objdre:" << objND << "\n";
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

      if (objND && Kind != K::I_OBJ) {
        Kind = K::W_OBJ;
      }
    }
  }

  void checkAlloca(const Expr* E) {
    if (const ParenExpr* PE = dyn_cast_or_null<ParenExpr>(E)) {
      if (const NamedDecl* ND = getNDFromPE(PE)) {
        const IdentifierInfo* II = ND->getIdentifier();

        if (II && inAlloc(II)) {
          // obj assignment
          Kind = K::I_OBJ;
        }
      }
    }
  }

public:
  AssignmentWalker(const Stmt* S)
      : Kind(K::NONE), objND(nullptr), fieldND(nullptr) {
    if (const BinaryOperator* BO = dyn_cast_or_null<BinaryOperator>(S)) {
      analyzeBO(BO);
    }
  }

  const NamedDecl* getObjND() { return (const NamedDecl*)objND; }

  const NamedDecl* getFieldND() { return (const NamedDecl*)fieldND; }

  bool isWriteObj() { return Kind == K::W_OBJ; }

  bool isWriteField() { return Kind == K::W_FIELD; }

  bool isInitObj() { return Kind == K::I_OBJ; }

  bool isUsed() { return Kind != K::NONE; }

  void analyzeBO(const BinaryOperator* BO) {
    if (BO->isAssignmentOp()) {
      Expr* LHS = BO->getLHS();
      Expr* RHS = BO->getRHS();

      // for checking if it is alloca
      checkAlloca(RHS);

      // for capturing obj and fields
      extractNDs(LHS);
    }
  }
};

} // namespace clang::ento::nvm