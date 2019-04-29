#pragma once
#include "Common.h"
#include "PersistentGetters.h"

namespace clang::ento::nvm {

class IPWalker {
  enum K { FIELD, OBJ, NONE } Kind;
  const ASTContext& ASTC;
  NamedDecl* objND;
  NamedDecl* fieldND;

public:
  IPWalker(const Expr* E, const ASTContext& ASTC_)
      : Kind(K::NONE), ASTC(ASTC_), objND(nullptr), fieldND(nullptr) {
    if (const ImplicitCastExpr* ICE = dyn_cast_or_null<ImplicitCastExpr>(E)) {
      analyzeICE(ICE);
    }
  }

  const NamedDecl* getObjND() { return (const NamedDecl*)objND; }
  const NamedDecl* getFieldND() { return (const NamedDecl*)fieldND; }

  void analyzeICE(const ImplicitCastExpr* ICE) {
    if (!ICE) {
      return;
    }

    QualType QT = ICE->getType();
    //do not track non persistent types
    if(!isPersistentType(QT, ASTC)){
      return;
    }

    const Stmt* Child1 = getNthChild(ICE, 0);
    if (const DeclRefExpr* DRE = dyn_cast_or_null<DeclRefExpr>(Child1)) {
      // obj
      objND = (NamedDecl*)DRE->getFoundDecl();
      Kind = K::OBJ;
    } else if (const MemberExpr* ME = dyn_cast_or_null<MemberExpr>(Child1)) {
      // field
      fieldND = (NamedDecl*)ME->getMemberDecl();
      Kind = K::FIELD;
    }
  }

  bool isNone() const { return Kind == K::NONE; }

  bool isObj() const { return Kind == K::OBJ; }

  bool isField() const { return Kind == K::FIELD; }
};

} // namespace clang::ento::nvm