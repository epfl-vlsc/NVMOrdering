#pragma once
#include "clang/AST/StmtVisitor.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerRegistry.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "llvm/Support/raw_ostream.h"
#include <array>
#include <map>
#include <set>
#include <vector>
#include <utility>

#define dbg_assert(map, key, msg)                                              \
  if (!map.count(key)) {                                                       \
    llvm::errs() << key << " does not exist\n";                                \
    assert(0 && msg);                                                          \
  }

namespace clang::ento::nvm {

const AnnotateAttr* getAnnotation(const ValueDecl* VD) {
  for (const auto* Ann : VD->specific_attrs<AnnotateAttr>()) {
    // add to annotated vars
    return Ann;
  }

  return nullptr;
}

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

const ValueDecl* getValueDecl(const Decl* BD) {
  if (const ValueDecl* D = dyn_cast_or_null<ValueDecl>(BD)) {
    return D;
  }
  llvm::report_fatal_error("must have value decl");
  return nullptr;
}

const ValueDecl* getValueDecl(const SVal& Loc) {
  const MemRegion* Region = Loc.getAsRegion();
  if (const FieldRegion* FieldReg = Region->getAs<FieldRegion>()) {
    const Decl* BD = FieldReg->getDecl();
    const ValueDecl* VD = getValueDecl(BD);
    return VD;
  }

  return nullptr;
}

const ValueDecl* getValueDecl(const char* D){
  const Decl* BD = (const Decl*)D;
  return getValueDecl(BD);
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

} // namespace clang::ento::nvm