#pragma once
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/StmtVisitor.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Frontend/CheckerRegistry.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "llvm/Support/raw_ostream.h"
#include <array>
#include <list>
#include <map>
#include <set>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#define dbg_assert(map, key, msg)                                              \
  if (!map.count(key)) {                                                       \
    llvm::errs() << key << " does not exist\n";                                \
    assert(0 && msg);                                                          \
  }

namespace clang::ento::nvm {

const MemRegion* getTopBaseMemRegion(const MemRegion* MR) {
  assert(MR && "must be valid MemReg");
  const MemRegion* BMR = MR->getBaseRegion();
  if (BMR == MR || !BMR) {
    return MR;
  } else {
    return getTopBaseMemRegion(BMR);
  }
}

const MemRegion* getTopBaseMemRegUnsafe(const SVal& Loc) {
  const MemRegion* LocMR = Loc.getAsRegion();
  if (!LocMR) {
    return nullptr;
  }

  const MemRegion* LocBMR = getTopBaseMemRegion(LocMR);
  return LocBMR;
}

const MemRegion* getTopBaseMemRegLOrR(const SVal& L, const SVal& R, bool l) {
  if (l) {
    return getTopBaseMemRegUnsafe(L);
  } else {
    return getTopBaseMemRegUnsafe(R);
  }
}

bool isFieldOrObj(const NamedDecl* ND) {
  if (const FieldDecl* FD = dyn_cast_or_null<FieldDecl>(ND)) {
    return true;
  } else if (const RecordDecl* RD = dyn_cast_or_null<RecordDecl>(ND)) {
    return true;
  }
  return false;
}

const FieldDecl* getFieldFromNDs(const NamedDecl* ND1, const NamedDecl* ND2) {
  if (const FieldDecl* FD = dyn_cast_or_null<FieldDecl>(ND1)) {
    return FD;
  } else if (const FieldDecl* FD = dyn_cast_or_null<FieldDecl>(ND2)) {
    return FD;
  }
  llvm::report_fatal_error("must be field");
  return nullptr;
}

const RecordDecl* getRecordDeclFromND(const NamedDecl* ND) {
  if (const RecordDecl* RD = dyn_cast_or_null<RecordDecl>(ND)) {
    return RD;
  }

  return nullptr;
}

unsigned getSrcLineNo(const SourceManager& SM, const SourceLocation& SL) {
  auto [FID, Pos] = SM.getDecomposedLoc(SL);
  unsigned lineNo = SM.getLineNumber(FID, Pos);
  return lineNo;
}

const Stmt* getNthChild(const Stmt* S, int i) {
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

const AnnotateAttr* getAnnotation(const ValueDecl* VD) {
  for (const auto* Ann : VD->specific_attrs<AnnotateAttr>()) {
    // add to annotated vars
    return Ann;
  }

  return nullptr;
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

const RecordDecl* getRDFromAny(const NamedDecl* ND) {
  if (const FieldDecl* FD = dyn_cast<FieldDecl>(ND)) {
    const RecordDecl* RD = FD->getParent();
    return RD;
  } else if (const RecordDecl* RD = dyn_cast<RecordDecl>(ND)) {
    return RD;
  }

  return nullptr;
}

bool isRDAligned(const RecordDecl* RD) {
  assert(RD);

  return RD->hasAttr<AlignedAttr>();
}

const FunctionDecl* getFuncDecl(const Decl* BD) {
  if (const FunctionDecl* D = dyn_cast_or_null<FunctionDecl>(BD)) {
    return D;
  }
  return nullptr;
}

struct StmtOrDecl {
  // use this instead of dynamic node
private:
  const Stmt* S;
  const Decl* D;

  bool isS;
  bool isD;

public:
  StmtOrDecl(const Stmt* S_) : S(S_), D(nullptr), isS(true), isD(false) {}
  StmtOrDecl(const Decl* D_) : S(nullptr), D(D_), isS(false), isD(true) {}
  StmtOrDecl() : S(nullptr), D(nullptr), isS(false), isD(false) {}

  bool isStmt() const { return isS; }

  bool isDecl() const { return isD; }

  const Stmt* getS() const { return S; }
  const Decl* getD() const { return D; }

  void dump() const {
    if (isS) {
      llvm::errs() << "stmt: ";
      S->dump();
    } else if (isD) {
      llvm::errs() << "decl: ";
      D->dump();
    } else {
      llvm::errs() << "nothing";
    }
    llvm::errs() << "\n";
  }
};

template <typename P> StmtOrDecl getParentStmtOrDecl(P& parents) {
  if (parents.empty()) {
    llvm::report_fatal_error("cannot find parent");
    return StmtOrDecl();
  } else if (const Stmt* PS = parents[0].template get<Stmt>()) {
    return StmtOrDecl(PS);
  } else if (const Decl* PD = parents[0].template get<Decl>()) {
    return StmtOrDecl(PD);
  } else {
    return StmtOrDecl();
  }
}

StmtOrDecl getParentStmtOrDecl(StmtOrDecl& SorD, ASTContext& AC) {
  if (SorD.isStmt()) {
    const Stmt* S = SorD.getS();
    const auto& parents = AC.getParents(*S);
    return getParentStmtOrDecl(parents);
  } else if (SorD.isDecl()) {
    const Decl* D = SorD.getD();
    const auto& parents = AC.getParents(*D);
    return getParentStmtOrDecl(parents);
  } else {
    return StmtOrDecl();
  }
}

const FunctionDecl* getFuncDecl(StmtOrDecl& SorD, ASTContext& AC) {
  StmtOrDecl PSorD = getParentStmtOrDecl(SorD, AC);
  if (PSorD.isStmt()) {
    return getFuncDecl(PSorD, AC);
  } else if (PSorD.isDecl()) {
    const Decl* D = PSorD.getD();
    if (const FunctionDecl* FD = dyn_cast_or_null<FunctionDecl>(D)) {
      // get the first top function
      return FD;
    }
    return getFuncDecl(PSorD, AC);
  } else {
    llvm::report_fatal_error("member must be used within function");
    return nullptr;
  }
}

const FunctionDecl* getFuncDecl(const Decl* D, ASTContext& AC) {
  StmtOrDecl SorD(D);
  return getFuncDecl(SorD, AC);
}

const FunctionDecl* getFuncDecl(const Stmt* S, ASTContext& AC) {
  StmtOrDecl SorD(S);
  return getFuncDecl(SorD, AC);
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

const FunctionDecl* getCallerFuncDecl(CheckerContext& C) {
  return getFuncDecl(C);
}

const LocationContext* getCallerLocationContext(CheckerContext& C) {
  const LocationContext* LC = C.getLocationContext();
  return LC->getParent();
}

bool isCallerParentOfCallee(const LocationContext* CallerLC,
                            const LocationContext* CalleeLC) {
  if (!CallerLC) {
    // top level is nullptr
    return true;
  } else {
    return CallerLC->isParentOf(CalleeLC);
  }
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

const ValueDecl* getValueDecl(const MemRegion* Region) {
  if (!Region) {
    return nullptr;
  } else if (const FieldRegion* FieldReg = Region->getAs<FieldRegion>()) {
    const Decl* BD = FieldReg->getDecl();
    const ValueDecl* VD = getValueDecl(BD);
    return VD;
  }

  return nullptr;
}

const ValueDecl* getValueDecl(const SVal& Loc) {
  const MemRegion* Region = Loc.getAsRegion();
  return getValueDecl(Region);
}

const ValueDecl* getValueDecl(const char* D) {
  if (!D) {
    return nullptr;
  }
  const Decl* BD = (const Decl*)D;
  return getValueDecl(BD);
}

const FieldDecl* getMemFieldDecl(const MemRegion* Region) {
  if (!Region) {
    return nullptr;
  } else if (const FieldRegion* FieldReg = Region->getAs<FieldRegion>()) {
    const Decl* BD = FieldReg->getDecl();
    if (const FieldDecl* FD = dyn_cast_or_null<FieldDecl>(BD)) {
      return FD;
    }
  } else if (const SymbolicRegion* SymReg = Region->getAs<SymbolicRegion>()) {
    SymbolRef SR = SymReg->getSymbol();
    const MemRegion* FMR = SR->getOriginRegion();
    if (const FieldDecl* FD = getMemFieldDecl(FMR); FD) {
      return FD;
    }
  }

  return nullptr;
}

const FieldDecl* getMemFieldDecl(const SVal& Loc) {
  const MemRegion* Region = Loc.getAsRegion();
  return getMemFieldDecl(Region);
}

const FieldDecl* getMemLogFieldDecl(const SVal& Loc) {
  return getMemFieldDecl(Loc);
}

const RecordDecl* getMemRecordDecl(const MemRegion* Region) {
  if (!Region) {
    return nullptr;
  } else if (const SymbolicRegion* ObjReg = Region->getAs<SymbolicRegion>()) {
    SymbolRef SR = ObjReg->getSymbol();
    QualType QT = SR->getType();
    const Type* T = QT.getTypePtr();
    if (T->isPointerType()) {
      QualType RQT = T->getPointeeType();
      const Type* RT = RQT.getTypePtr();
      if (const RecordType* CRT = RT->getAs<RecordType>()) {
        const RecordDecl* RD = CRT->getDecl();
        return RD;
      }
    }
  }

  return nullptr;
}

const RecordDecl* getMemRecordDecl(const SVal& Loc) {
  const MemRegion* Region = Loc.getAsRegion();
  return getMemRecordDecl(Region);
}

const VarDecl* getVarDecl(const MemRegion* ParentRegion) {
  if (!ParentRegion) {
    return nullptr;
  } else if (const VarRegion* VarReg = ParentRegion->getAs<VarRegion>()) {
    const VarDecl* VD = VarReg->getDecl();
    return VD;
  }
  return nullptr;
}

bool isPtrRegion(const VarRegion* VarReg) {
  if (!VarReg) {
    return false;
  }
  QualType QT = VarReg->getValueType();
  const Type* T = QT.getTypePtr();
  return T->isPointerType();
}

const VarDecl* getVDFromVarReg(const VarRegion* VarReg) {
  return VarReg->getDecl();
}

const RecordDecl* getRecordDecl(QualType& QT) {
  const Type* type = QT.getTypePtr();
  if (const RecordDecl* RD =
          dyn_cast_or_null<RecordDecl>(type->getAsTagDecl())) {
    return RD;
  }
  return nullptr;
}

const NamedDecl* getNamedDecl(const MemRegion* Region) {
  if (const FieldRegion* FieldReg = Region->getAs<FieldRegion>()) {
    const Decl* BD = FieldReg->getDecl();
    const ValueDecl* VD = getValueDecl(BD);
    return VD;
  } else if (const TypedValueRegion* TypedValueReg =
                 Region->getAs<TypedValueRegion>()) {
    QualType QT = TypedValueReg->getValueType();
    const RecordDecl* RD = getRecordDecl(QT);
    return RD;
  }

  return nullptr;
}

const VarDecl* getVDUsingOrigin(SVal Loc) {
  if (const SymExpr* SE = Loc.getAsSymbolicExpression()) {
    const MemRegion* Region = SE->getOriginRegion();
    const VarDecl* VD = getVarDecl(Region);
    return VD;
  }
  return nullptr;
}

const VarDecl* getVDUsingLazy(SVal Loc) {
  if (!Loc.isUnknownOrUndef()) {
    nonloc::LazyCompoundVal LCV = Loc.castAs<nonloc::LazyCompoundVal>();
    const TypedValueRegion* CurrentRegion = LCV.getRegion();
    if (CurrentRegion) {
      const MemRegion* ParentRegion = CurrentRegion->getBaseRegion();
      const VarDecl* Obj = getVarDecl(ParentRegion);
      return Obj;
    }
  }
  return nullptr;
}

const MemRegion* getTopBaseRegion(SVal Loc) {
  // todo return top, currently returns parent
  const MemRegion* CurrentRegion = Loc.getAsRegion();
  if (!CurrentRegion) {
    return CurrentRegion;
  }

  const MemRegion* ParentRegion = CurrentRegion->getBaseRegion();
  return ParentRegion;
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

std::string getFunctionDeclName(CheckerContext& C) {
  const FunctionDecl* FD = getFuncDecl(C);
  return FD->getQualifiedNameAsString();
}

} // namespace clang::ento::nvm