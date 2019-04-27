#pragma once
#include "Common.h"
#include "FncNames.h"
#include "PersistentGetters.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace clang::ento::nvm {

// todo handle write to obj

bool isPersistentType(const QualType& QT, const ASTContext& ASTC) {
  QualType PT = QT.getDesugaredType(ASTC);
  if (const IdentifierInfo* II = PT.getBaseTypeIdentifier()) {
    if (II->getName().contains("toid")) {
      llvm::errs() << "persistent"
                   << "\n";
      return true;
    }
  }
  return false;
}

class AssignmentWalker {
  enum K { W_OBJ, W_FIELD, I_OBJ, NONE } Kind;
  const ASTContext& ASTC;
  NamedDecl* objND;
  NamedDecl* fieldND;

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

  void setFieldWrite(const MemberExpr* ME) {
    if (const VarDecl* VD = getVDFromME(ME)) {
      QualType QT = VD->getType();
      if (isPersistentType(QT, ASTC) && objND && fieldND) {
        Kind = K::W_FIELD;
      }
    }
  }

  void setObjWrite(const UnaryOperator* UO) {
    UO->dump();
    QualType QT = UO->getType();
    QT.dump();
    if (isPersistentType(QT, ASTC) && objND) {
      if (Kind == K::NONE) {
        // if it is a write
        Kind = K::W_OBJ;
      }
    } else {
      // obj does not exist
      Kind = K::NONE;
    }
  }

  void extractNDs(const Expr* E) {
    if (const MemberExpr* ME = dyn_cast_or_null<MemberExpr>(E)) {
      // possible field write
      // field
      setFieldME(ME);

      // obj
      setObjME(ME);

      // check if persistent and uses field
      setFieldWrite(ME);
    } else if (const UnaryOperator* UO = dyn_cast_or_null<UnaryOperator>(E)) {
      // extract obj
      setObjUO(UO);

      // check if persistent and uses field
      setObjWrite(UO);
    }
  }

  void setAllocaWrite(const Expr* E) {
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
  AssignmentWalker(const Stmt* S, const ASTContext& ASTC_)
      : Kind(K::NONE), ASTC(ASTC_), objND(nullptr), fieldND(nullptr) {
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

      // check if alloca is used
      setAllocaWrite(RHS);

      // for capturing obj and fields
      extractNDs(LHS);
    }
  }
};

} // namespace clang::ento::nvm