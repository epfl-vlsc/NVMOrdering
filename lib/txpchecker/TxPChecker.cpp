//===-- TxPChecker.cpp -----------------------------------------*

#include "TxPChecker.h"

namespace clang::ento::nvm {

void TxPChecker::checkBeginFunction(CheckerContext& C) const {
  bool isPFnc = nvmTxInfo.isPFunction(C);
  bool isTopFnc = isTopFunction(C);

  // if pmalloc/pfree/paccess function, do not analyze
  if (isPFnc && isTopFnc) {
    ExplodedNode* ErrNode = C.generateErrorNode();
    if (!ErrNode)
      return;
  }
}

void TxPChecker::checkEndFunction(CheckerContext& C) const {
  /*
  ProgramStateRef State = C.getState();
  bool isTopFnc = isTopFunction(C);

  // if pmalloc/pfree/paccess function, do not analyze
  if (isTopFnc) {
    printStates<PMap>(State, C);
  }
  */
}
template <typename SMap>
void TxPChecker::printStates(ProgramStateRef& State, CheckerContext& C) const {
  DBG("printStates")
  for (auto& [D, SS] : State->get<SMap>()) {
    llvm::outs() << (void*)D << " " << SS.getStateName() << "\n";
  }
}

void TxPChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                           CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  /*
  llvm::errs() << "Bind" << "\n";
  S->dump();
  llvm::errs() << "\n";
  S->dumpPretty(C.getASTContext());
  llvm::errs() << "\n";
  Loc.dump();
  llvm::errs() << "\n";

  if(Loc.isUnknown()){
    llvm::errs() << "unk\n";
  }
  */

  // add field

  /*
  auto RI =
      ReportInfos::getRI(C, State, CurrentVD, BaseVD, BReporter, &Loc, S);
  writeDirect(RI);
  stateChanged |= RI.stateChanged;
  */

  addStateTransition(State, C, stateChanged);
}

void TxPChecker::checkASTDecl(const FunctionDecl* FD, AnalysisManager& Mgr,
                              BugReporter& BR) const {
  nvmTxInfo.insertFunction(FD);
}

void TxPChecker::checkPostCall(const CallEvent& Call, CheckerContext& C) const {
  const FunctionDecl* FD = getFuncDecl(Call);
  if (!FD) {
    return;
  }

  if (nvmTxInfo.isTxBeg(FD)) {
    // handleTxBegin(C);
  } else if (nvmTxInfo.isTxEnd(FD)) {
    // handleTxEnd(C);
  } else if (nvmTxInfo.isPalloc(FD)) {
    // handlePalloc(Call, C);
  } else if (nvmTxInfo.isPfree(FD)) {
    // handlePfree(Call, C);
  } else if (nvmTxInfo.isPacc(FD)) {
    handlePacc(Call, C);
  } else if (nvmTxInfo.isTxRange(FD)) {
    handleTxRange(Call, C);
  } else if (nvmTxInfo.isTxRangeDirect(FD)) {
    handleTxRangeDirect(Call, C);
  } else {
    // nothing
  }
}

void TxPChecker::handlePacc(const CallEvent& Call, CheckerContext& C) const {
  SVal Loc = Call.getArgSVal(0);
  /*
  llvm::errs() << "Pmemdirect"
               << "\n";
  */

  if (!Loc.isUnknownOrUndef()) {
    nonloc::LazyCompoundVal LCV = Loc.castAs<nonloc::LazyCompoundVal>();
    const TypedValueRegion* CurrentRegion = LCV.getRegion();
    if (CurrentRegion) {
      const MemRegion* ParentRegion = CurrentRegion->getBaseRegion();
      const VarDecl* VD = getVarDecl(ParentRegion);

      /*
      if (VD)
        llvm::outs() << VD->getNameAsString() << "\n";
      */
    }
  }
}

void TxPChecker::handleTxRangeDirect(const CallEvent& Call,
                                     CheckerContext& C) const {
  SVal Loc = Call.getArgSVal(0);
  /*
  llvm::errs() << "RangeDirect"
               << "\n";
  */

  if (const SymExpr* SE = Loc.getAsSymbolicExpression()) {
    const MemRegion* Region = SE->getOriginRegion();
    const VarDecl* VD = getVarDecl(Region);

    /*
     if (VD)
       llvm::outs() << VD->getNameAsString() << "\n";
     */
  }
}

void TxPChecker::handleTxRange(const CallEvent& Call, CheckerContext& C) const {

  llvm::errs() << "Range"
               << "\n";
  SourceRange SR = Call.getSourceRange();
  SourceLocation SL = SR.getBegin();
  SL.dump(C.getSourceManager());
  llvm::errs() << "\n";

  SVal Obj = Call.getArgSVal(0);
  const Expr* Field = Call.getArgExpr(1);

  Field->dump();
  llvm::errs() << "\n";

  //get obj
  if (!Obj.isUnknownOrUndef()) {
    nonloc::LazyCompoundVal LCV = Obj.castAs<nonloc::LazyCompoundVal>();
    const TypedValueRegion* CurrentRegion = LCV.getRegion();
    if (CurrentRegion) {
      CurrentRegion->dump();
      llvm::errs() << "\n";

      const MemRegion* ParentRegion = CurrentRegion->getBaseRegion();
      const VarDecl* VD = getVarDecl(ParentRegion);

      /*
      if (VD){
        llvm::errs() << "VD: " << VD->getNameAsString() << "\n";
      }
      */
    }
  }
}

void TxPChecker::handlePalloc(const CallEvent& Call, CheckerContext& C) const {
  /*
  // taint
  ProgramStateRef State = C.getState();
  SVal RV = Call.getReturnValue();
  const MemRegion* Region = RV.getAsRegion();

  // taint regions
  // todo might lead to bugs due to underlying implementation, this is not how
  // it is used

  unsigned txCount = State->get<TxCounter>();
  if (txCount == 0) {
    ExplodedNode* ErrNode = C.generateNonFatalErrorNode();
    if (!ErrNode) {
      return;
    }
    BReporter.reportTxWriteBug(Region, C, ErrNode, C.getBugReporter());
  }

  State = State->set<PMap>(Region, WriteState::getInit());
  C.addTransition(State);
  */
}

void TxPChecker::handlePfree(const CallEvent& Call, CheckerContext& C) const {
  /*
  if (Call.getNumArgs() > 1) {
    llvm::report_fatal_error("check pfree function");
    return;
  }

  ProgramStateRef State = C.getState();
  SVal Loc = Call.getArgSVal(0);
  const MemRegion* Region = Loc.getAsRegion();

  const MemRegion* BR = Region->getBaseRegion();

  bool isNvm = State->get<PMap>(BR);
  if (isNvm) {
    // check transaction
    unsigned txCount = State->get<TxCounter>();
    if (txCount == 0) {
      ExplodedNode* ErrNode = C.generateNonFatalErrorNode();
      if (!ErrNode) {
        return;
      }
      BReporter.reportTxWriteBug(BR, C, ErrNode, C.getBugReporter());
    }

    // clear the region
    State = State->remove<PMap>(BR);
    C.addTransition(State);
  }
  */
}

void TxPChecker::handleTxBegin(CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  unsigned txCount = State->get<TxCounter>();
  txCount += 1;
  State = State->set<TxCounter>(txCount);
  C.addTransition(State);
}

void TxPChecker::handleTxEnd(CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  unsigned txCount = State->get<TxCounter>();
  if (txCount) {
    // todo report bug that begin does not match end
  }
  txCount -= 1;
  State = State->set<TxCounter>(txCount);
  C.addTransition(State);
}

void TxPChecker::addStateTransition(ProgramStateRef& State, CheckerContext& C,
                                    bool stateChanged) const {
  if (stateChanged) {
    DBG("state transition")
    C.addTransition(State);
  }
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::TxPChecker>(CHECKER_PLUGIN_NAME,
                                                    "Checks pmdk");
}
