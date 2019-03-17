#pragma once
#include "clang/AST/StmtVisitor.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerRegistry.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "llvm/Support/raw_ostream.h"
#include <utility>

namespace clang::ento::nvm {

const FunctionDecl* getTopFunction(CheckerContext& C) {
  LocationContext* LC = (LocationContext*)C.getLocationContext();
  while (LC && LC->getParent()) {
    LC = (LocationContext*)LC->getParent();
  }

  const Decl* BD = LC->getDecl();
  if (const FunctionDecl* FD = dyn_cast<FunctionDecl>(BD)) {
    return FD;
  } else {
    llvm::report_fatal_error("always stack function");
    return nullptr;
  }
}

const FunctionDecl* getFuncDecl(const Decl* BD) {
  if (const FunctionDecl* D = dyn_cast_or_null<FunctionDecl>(BD)) {
    return D;
  }
  llvm::report_fatal_error("use only for functions");
  return nullptr;
}

const FunctionDecl* getFuncDecl(const CallEvent& Call) {
  const Decl* BD = Call.getDecl();
  return getFuncDecl(BD);
}

const FunctionDecl* getFuncDecl(const LocationContext* LC) {
  const Decl* BD = LC->getDecl();
  return getFuncDecl(BD);
}

const FunctionDecl* getFuncDecl(CheckerContext& C) {
  const LocationContext* LC = C.getLocationContext();
  const Decl* BD = LC->getDecl();
  return getFuncDecl(BD);
}

const DeclaratorDecl* getDeclaratorDecl(const Decl* BD) {
  if (const DeclaratorDecl* D = dyn_cast_or_null<DeclaratorDecl>(BD)) {
    return D;
  }
  llvm::report_fatal_error("All calls have function declaration");
  return nullptr;
}

bool isTopFunction(CheckerContext& C) { return C.inTopFrame(); }

class FieldWalker : public ConstStmtVisitor<FieldWalker> {
  std::set<const DeclaratorDecl*> fieldSet;

public:
  void VisitStmt(const Stmt* S) { VisitChildren(S); }

  void VisitChildren(const Stmt* S) {
    for (Stmt::const_child_iterator I = S->child_begin(), E = S->child_end();
         I != E; ++I) {
      if (const Stmt* Child = *I) {
        Visit(Child);
      }
    }
  }

  void VisitMemberExpr(const MemberExpr* ME) {
    const ValueDecl* VD = ME->getMemberDecl();
    if (const DeclaratorDecl* DD = dyn_cast<DeclaratorDecl>(VD)) {
      fieldSet.insert(DD);
    } else {
      llvm::report_fatal_error("only annotate proper fields");
    }
  }

  std::set<const DeclaratorDecl*>& getFieldSet() { return fieldSet; }
};

class MaskWalker : public ConstStmtVisitor<MaskWalker> {
  StringRef mask;
  bool usesMask;
  bool writeMode;

public:
  void VisitDeclRefExpr(const DeclRefExpr* DRE) {
    StringRef currentMask =
        DRE->getNameInfo().getName().getAsIdentifierInfo()->getName();
    if (currentMask.equals(mask)) {
      usesMask = true;
    }
  }

  MaskWalker(StringRef mask_, bool writeMode_)
      : mask(mask_), usesMask(false), writeMode(writeMode_) {}

  void VisitStmt(const Stmt* S) { VisitChildren(S); }

  void VisitUnaryOperator(const UnaryOperator* UOp) {
    // using another mask
    usesMask = false;
  }

  void VisitChildren(const Stmt* S) {
    for (Stmt::const_child_iterator I = S->child_begin(), E = S->child_end();
         I != E; ++I) {
      if (const Stmt* Child = *I) {
        Visit(Child);
      }
    }
  }

  bool useMask() { return usesMask; }
};

bool usesMask(const Stmt* S, StringRef mask, bool writeMode = true) {
  MaskWalker maskWalker(mask, writeMode);
  maskWalker.Visit(S);
  return maskWalker.useMask();
}

} // namespace clang::ento::nvm