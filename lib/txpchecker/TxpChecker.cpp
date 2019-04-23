//===-- TxPChecker.cpp -----------------------------------------*

#include "TxPChecker.h"
#include "FieldWalkers.h"

namespace clang::ento::nvm {

void TxPChecker::checkBeginFunction(CheckerContext& C) const {
  bool isPFnc = nvmTxInfo.isPFunction(C);
  bool isTopFnc = isTopFunction(C);
  bool isAnalyzeFnc = nvmTxInfo.isAnnotatedFnc(C);

  // if pmalloc/pfree/paccess function, do not analyze
  if ((isPFnc || !isAnalyzeFnc) && isTopFnc) {
    handleEnd(C);
  }
}

void TxPChecker::handleEnd(CheckerContext& C) const {
  DBG("handleEnd")
  ExplodedNode* ErrNode = C.generateErrorNode();
  if (!ErrNode)
    return;
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
  DBG("Bind")
  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  AssignmentWalker aw;
  aw.TraverseStmt((Stmt*)S);
  if (aw.isObjWrite()) {
    DBG("obj write")

    // alloc of object
    auto RI = ReportInfos::getRI(C, State, aw.getObjInfo(), nullptr, BReporter,
                                 &Loc, S);
    if (!inTx(State)) {
      // not done in transaction
      RI.reportWriteOutTxBug();
    } else {
      // done in transaction
      Transitions::pdirectAccess(RI);
      stateChanged |= RI.stateChanged;
    }

  } else if (aw.isFieldWrite()) {
    DBG("field write")

    // write to obj field

    auto RI = ReportInfos::getRI(C, State, aw.getObjInfo(), aw.getFieldInfo(),
                                 BReporter, &Loc, S);
    if (!inTx(State)) {
      // not done in transaction
      RI.reportWriteOutTxBug();
    } else {
      // done in transaction
      Transitions::writeData(RI);
      stateChanged |= RI.stateChanged;
    }
  }

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

  DBG("checkPostCall:" << FD->getName())

  if (nvmTxInfo.isTxBeg(FD)) {
    handleTxBegin(Call, C);
  } else if (nvmTxInfo.isTxEnd(FD)) {
    handleTxEnd(Call, C);
  } else if (nvmTxInfo.isPalloc(FD)) {
    // handlePalloc(Call, C);
  } else if (nvmTxInfo.isPfree(FD)) {
    // handlePfree(Call, C);
  } else if (nvmTxInfo.isPdirect(FD)) {
    handlePdirect(Call, C);
  } else if (nvmTxInfo.isTxRange(FD)) {
    handleTxRange(Call, C);
  } else if (nvmTxInfo.isTxRangeDirect(FD)) {
    handleTxRangeDirect(Call, C);
  } else {
    // nothing
  }
}

void TxPChecker::handlePdirect(const CallEvent& Call, CheckerContext& C) const {
  DBG("handlePdirect")
  SVal Loc = Call.getArgSVal(0);

  if (!Loc.isUnknownOrUndef()) {
    nonloc::LazyCompoundVal LCV = Loc.castAs<nonloc::LazyCompoundVal>();
    const TypedValueRegion* CurrentRegion = LCV.getRegion();
    if (CurrentRegion) {
      const MemRegion* ParentRegion = CurrentRegion->getBaseRegion();
      const VarDecl* VD = getVarDecl(ParentRegion);

      if (VD) {
        // obj access
        ProgramStateRef State = C.getState();
        bool stateChanged = false;

        auto RI =
            ReportInfos::getRI(C, State, VD, nullptr, BReporter, &Loc, nullptr);
        if (!inTx(State)) {
          // not done in transaction
          RI.reportWriteOutTxBug();
        } else {
          // done in transaction
          Transitions::pdirectAccess(RI);
          stateChanged |= RI.stateChanged;
        }

        addStateTransition(State, C, stateChanged);
      }
    }
  }
}

void TxPChecker::handleTxRangeDirect(const CallEvent& Call,
                                     CheckerContext& C) const {
  DBG("handleTxRangeDirect")
  SVal Loc = Call.getArgSVal(0);

  if (const SymExpr* SE = Loc.getAsSymbolicExpression()) {
    const MemRegion* Region = SE->getOriginRegion();
    const VarDecl* VD = getVarDecl(Region);

    if (VD) {
      ProgramStateRef State = C.getState();
      bool stateChanged = false;
      auto RI = ReportInfos::getRI(C, State, VD, nullptr, BReporter, nullptr,
                                   nullptr);

      if (!inTx(State)) {
        // not done in transaction
        RI.reportWriteOutTxBug();
      } else {
        // done in transaction
        Transitions::logData(RI);
        stateChanged |= RI.stateChanged;
      }

      addStateTransition(State, C, stateChanged);
    }
  }
}

void TxPChecker::handleTxRange(const CallEvent& Call, CheckerContext& C) const {
  DBG("handleTxRange")
  SVal Obj = Call.getArgSVal(0);
  const Expr* Field = Call.getArgExpr(1);
  RangeWalker rw;
  rw.TraverseStmt((Expr*)Field);

  // get obj
  if (!Obj.isUnknownOrUndef()) {
    nonloc::LazyCompoundVal LCV = Obj.castAs<nonloc::LazyCompoundVal>();
    const TypedValueRegion* CurrentRegion = LCV.getRegion();
    if (CurrentRegion) {
      const MemRegion* ParentRegion = CurrentRegion->getBaseRegion();
      const VarDecl* ObjD = getVarDecl(ParentRegion);
      const ValueDecl* FieldD = rw.getVD();

      if (ObjD) {
        // obj or field accessed
        ProgramStateRef State = C.getState();
        bool stateChanged = false;
        auto RI = ReportInfos::getRI(C, State, ObjD, FieldD, BReporter, nullptr,
                                     nullptr);

        if (!inTx(State)) {
          // not done in transaction
          RI.reportWriteOutTxBug();
        } else {
          // done in transaction
          Transitions::logData(RI);
          stateChanged |= RI.stateChanged;
        }

        addStateTransition(State, C, stateChanged);
      }
    }
  }
}

bool TxPChecker::inTx(ProgramStateRef& State) const {
  DBG("inTx")
  unsigned txCount = State->get<TxCounter>();
  return txCount > 0;
}

/*
void TxPChecker::handlePalloc(const CallEvent& Call, CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  SVal Loc = Call.getReturnValue();

  llvm::errs() << "Palloc" << "\n";
  Call.dump();
  llvm::errs() << "\n";
  Loc.dump();
  llvm::errs() << "\n";
  SourceRange SR = Call.getSourceRange();
  SourceLocation SL = SR.getBegin();
  SL.dump(C.getSourceManager());
  llvm::errs() << "\n";



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
}




void TxPChecker::handlePfree(const CallEvent& Call, CheckerContext& C) const {

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
}
*/

void TxPChecker::handleTxBegin(const CallEvent& Call, CheckerContext& C) const {
  DBG("handleTxBegin")
  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  auto RI = ReportInfos::getRI(C, State, nullptr, nullptr, BReporter, nullptr,
                               nullptr);
  Transitions::startTx(RI);
  stateChanged |= RI.stateChanged;

  addStateTransition(State, C, stateChanged);
}

void TxPChecker::handleTxEnd(const CallEvent& Call, CheckerContext& C) const {
  DBG("handleTxEnd")
  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  auto RI = ReportInfos::getRI(C, State, nullptr, nullptr, BReporter, nullptr,
                               nullptr);
  Transitions::endTx(RI);
  stateChanged |= RI.stateChanged;

  addStateTransition(State, C, stateChanged);
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
