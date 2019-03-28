#pragma once
#include "clang/AST/StmtVisitor.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerRegistry.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "llvm/Support/raw_ostream.h"
#include <utility>
#include <map>
#include <set>
#include <array>


#define dbg_assert(map, key, msg)                                             \
  if (!map.count(key)) {                                                      \
    llvm::errs() << key << " does not exist\n";                            \
    assert(0 && msg);                                                          \
  }

namespace clang::ento::nvm {

const Stmt* getParentStmt(const Stmt* S, CheckerContext& C) {
  ASTContext& AC = C.getASTContext();
  const auto& parents = AC.getParents(*S);
  if (parents.empty()) {
    llvm::report_fatal_error("cannot find parent");
    return nullptr;
  }

  const Stmt* PS = parents[0].get<Stmt>();
  if (!PS) {
    llvm::report_fatal_error("parent does not exist");
    return nullptr;
  }

  return PS;
}

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
  bool maskUse;
  bool usesUnary;
  bool readMode;

public:
  MaskWalker(StringRef mask_, bool readMode_)
      : mask(mask_), maskUse(false), usesUnary(false), readMode(readMode_) {}

  void VisitDeclRefExpr(const DeclRefExpr* DRE) {
    StringRef currentMask =
        DRE->getNameInfo().getName().getAsIdentifierInfo()->getName();
    // llvm::outs() << currentMask << " " << mask << "\n";
    if (currentMask.equals(mask)) {
      maskUse = true;
    }

    VisitChildren(DRE);
  }

  void VisitStmt(const Stmt* S) { VisitChildren(S); }

  void VisitUnaryOperator(const UnaryOperator* UOp) {
    // using another mask
    usesUnary = true;

    VisitChildren(UOp);
  }

  void VisitChildren(const Stmt* S) {
    for (Stmt::const_child_iterator I = S->child_begin(), E = S->child_end();
         I != E; ++I) {
      if (const Stmt* Child = *I) {
        Visit(Child);
      }
    }
  }

  bool usesMask() {
    if (!maskUse) {
      llvm::report_fatal_error("check mask usage");
    }

    return (usesUnary ^ readMode);
  }
};

bool usesMask(const Stmt* S, StringRef mask, bool readMode) {
  // S->dump();
  MaskWalker maskWalker(mask, readMode);
  maskWalker.Visit(S);
  return maskWalker.usesMask();
}

} // namespace clang::ento::nvm