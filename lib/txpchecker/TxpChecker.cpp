//===-- TxPChecker.cpp -----------------------------------------*

#include "TxpChecker.h"

namespace clang::ento::nvm {

void TxpChecker::checkBeginFunction(CheckerContext& C) const {
  const FunctionDecl* FD = getFuncDecl(C);
  bool isPFnc = txpFunctions.isPinit(FD);
  bool isAnalyzeFnc = txpFunctions.isAnnotatedFnc(FD);
  bool isTopFnc = isTopFunction(C);

  // if pmalloc/pfree/paccess function, do not analyze
  if ((isPFnc || !isAnalyzeFnc) && isTopFnc) {
    handleEnd(C);
  }
}

void TxpChecker::handleEnd(CheckerContext& C) const {
  DBG("handleEnd")
  ExplodedNode* ErrNode = C.generateErrorNode();
  if (!ErrNode)
    return;
}

void TxpChecker::checkEndFunction(CheckerContext& C) const {
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
void TxpChecker::printStates(ProgramStateRef& State, CheckerContext& C) const {
  DBG("printStates")
  for (auto& [D, SS] : State->get<SMap>()) {
    llvm::outs() << (void*)D << " " << SS.getStateName() << "\n";
  }
}

void TxpChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                           CheckerContext& C) const {
  DBG("Bind")
  ProgramStateRef State = C.getState();
  bool stateChanged = false;
  bool inTx = TxSpace::inTx(State);

  AssignmentWalker aw;
  aw.TraverseStmt((Stmt*)S);
  if (aw.isWriteObj()) {
    DBG("write obj")
    llvm::report_fatal_error("write to obj directly");
  } else if (aw.isWriteField()) {
    DBG("write field")
    auto SI = StateInfo(C, State, BReporter, &Loc, S, aw.getObjND(),
                        aw.getFieldND(), inTx);
    WriteSpace::writeField(SI);
    stateChanged |= SI.stateChanged;
  } else if (aw.isInitObj()) {
    DBG("alloc obj")
    auto SI =
        StateInfo(C, State, BReporter, &Loc, S, aw.getObjND(), nullptr, inTx);
    WriteSpace::writeObj(SI);
    stateChanged |= SI.stateChanged;
  }

  addStateTransition(State, C, stateChanged);
}

void TxpChecker::checkASTDecl(const FunctionDecl* FD, AnalysisManager& Mgr,
                              BugReporter& BR) const {
  txpFunctions.insertIfKnown(FD);
}

void TxpChecker::checkPreCall(const CallEvent& Call, CheckerContext& C) const {
  // interprocedural assignments
}

void TxpChecker::checkPostCall(const CallEvent& Call, CheckerContext& C) const {
  const FunctionDecl* FD = getFuncDecl(Call);

  if (!FD) {
    return;
  }

  DBG("checkPostCall:" << FD->getName())

  if (txpFunctions.isTxBeg(FD)) {
    handleTxBegin(Call, C);
  } else if (txpFunctions.isTxEnd(FD)) {
    handleTxEnd(Call, C);
  } else if (txpFunctions.isTxRange(FD)) {
    handleTxRange(Call, C);
  } else if (txpFunctions.isTxRangeDirect(FD)) {
    handleTxRangeDirect(Call, C);
  } else {
    // nothing
  }
}

void TxpChecker::handleTxRangeDirect(const CallEvent& Call,
                                     CheckerContext& C) const {
  DBG("handleTxRangeDirect")
  SVal Loc = Call.getArgSVal(0);
  if (const VarDecl* ObjVD = getVDUsingOrigin(Loc)) {
    DBG("obj log")
    ProgramStateRef State = C.getState();
    bool stateChanged = false;
    bool inTx = TxSpace::inTx(State);

    auto SI =
        StateInfo(C, State, BReporter, nullptr, nullptr, ObjVD, nullptr, inTx);
    WriteSpace::logObj(SI);
    stateChanged |= SI.stateChanged;
    addStateTransition(State, C, stateChanged);
  }
}

void TxpChecker::handleTxRange(const CallEvent& Call, CheckerContext& C) const {
  DBG("handleTxRange")
  // get obj
  SVal Loc = Call.getArgSVal(0);
  const VarDecl* ObjVD = getVDUsingLazy(Loc);

  // get field
  const Expr* Param1 = Call.getArgExpr(1);
  RangeWalker rw;
  rw.TraverseStmt((Expr*)Param1);
  const ValueDecl* FieldVD = rw.getVD();

  if (!ObjVD && !FieldVD) {
    llvm::report_fatal_error("tx_add_range has to have a proper argument");
    return;
  }

  ProgramStateRef State = C.getState();
  bool stateChanged = false;
  bool inTx = TxSpace::inTx(State);

  if (FieldVD) {
    DBG("field log")
    auto SI =
        StateInfo(C, State, BReporter, nullptr, nullptr, ObjVD, FieldVD, inTx);
    WriteSpace::logField(SI);
    stateChanged |= SI.stateChanged;
  } else if (ObjVD) {
    DBG("obj log")
    auto SI =
        StateInfo(C, State, BReporter, nullptr, nullptr, ObjVD, nullptr, inTx);
    WriteSpace::logObj(SI);
    stateChanged |= SI.stateChanged;
  }

  addStateTransition(State, C, stateChanged);
}

void TxpChecker::handleTxBegin(const CallEvent& Call, CheckerContext& C) const {
  DBG("handleTxBegin")
  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  auto SI =
      StateInfo(C, State, BReporter, nullptr, nullptr, nullptr, nullptr, false);
  TxSpace::begTx(SI);
  stateChanged |= SI.stateChanged;

  addStateTransition(State, C, stateChanged);
}

void TxpChecker::handleTxEnd(const CallEvent& Call, CheckerContext& C) const {
  DBG("handleTxEnd")
  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  auto SI =
      StateInfo(C, State, BReporter, nullptr, nullptr, nullptr, nullptr, false);
  TxSpace::endTx(SI);
  stateChanged |= SI.stateChanged;

  addStateTransition(State, C, stateChanged);
}

void TxpChecker::addStateTransition(ProgramStateRef& State, CheckerContext& C,
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
  registry.addChecker<clang::ento::nvm::TxpChecker>(CHECKER_PLUGIN_NAME,
                                                    "Checks pmdk");
}
