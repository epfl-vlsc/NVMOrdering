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

void TxpChecker::checkEndFunction(const ReturnStmt* RS,
                                  CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  bool isTopFnc = isTopFunction(C);

  // if pmalloc/pfree/paccess function, do not analyze
  if (isTopFnc) {
    // printStates(State, C);
  }
}

void TxpChecker::printStates(ProgramStateRef& State, CheckerContext& C) const {
  DBG("printStates")
  for (auto& [Param, Arg] : State->get<IpMap>()) {
    llvm::errs() << Param->getNameAsString() << " " << Arg->getNameAsString()
                 << "\n";
  }
}

void TxpChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                           CheckerContext& C) const {
  DBG("Bind")
  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  AssignmentWalker aw(S, C.getASTContext());

  if (aw.isWriteObj()) {
    DBG("write obj")
    const NamedDecl* ObjND = aw.getObjND();
    const NamedDecl* AliasND = aw.getRHSFromAss(S);

    IpSpace::setIpMap(State, ObjND, AliasND);
  } else if (aw.isWriteField()) {
    const NamedDecl* ObjND = IpSpace::getRealND(State, aw.getObjND());
    const NamedDecl* FieldND = IpSpace::getRealND(State, aw.getFieldND());
    assert(ObjND && FieldND && "obj and field has to exist");
    DBG("write field:" << FieldND->getNameAsString()
                       << "obj:" << ObjND->getNameAsString())
    auto SI = StateInfo(C, State, BReporter, S, ObjND, FieldND);
    TxSpace::checkTx(SI);
    WriteSpace::writeField(SI);
    stateChanged |= SI.stateChanged;
  } else if (aw.isInitObj()) {
    const NamedDecl* ObjND = IpSpace::getRealND(State, aw.getObjND());
    assert(ObjND && "obj has to exist");
    DBG("alloc obj:" << ObjND->getNameAsString())
    auto SI = StateInfo(C, State, BReporter, S, ObjND, nullptr);
    TxSpace::checkTx(SI);
    WriteSpace::writeObj(SI);
    stateChanged |= SI.stateChanged;
  }

  addStateTransition(State, C, stateChanged);
}

void TxpChecker::checkASTDecl(const FunctionDecl* FD, AnalysisManager& Mgr,
                              BugReporter& BR) const {
  txpFunctions.insertIfKnown(FD);
}

bool TxpChecker::evalCall(const CallExpr* CE, CheckerContext& C) const {
  // skip evaluation of all special pmdk function
  const FunctionDecl* FD = C.getCalleeDecl(CE);
  if (!FD) {
    return false;
  }

  if (txpFunctions.isAnyPfnc(FD)) {
    return true;
  }

  return false;
}

void TxpChecker::checkPreCall(const CallEvent& Call, CheckerContext& C) const {
  // todo remove mapping once the function is done
  const FunctionDecl* FD = getFuncDecl(Call);
  if (!FD || txpFunctions.isAnyPfnc(FD) || !Call.getNumArgs()) {
    return;
  }

  DBG("checkPreCall:" << FD->getName())
  ProgramStateRef State = C.getState();
  // interprocedural assignments
  int i = 0;
  for (const ParmVarDecl* Param : Call.parameters()) {
    const Expr* E = Call.getArgExpr(i);
    ++i;

    IPWalker ipw(E, C.getASTContext());
    if (ipw.isNone()) {
      continue;
    }

    // do interprocedural storage

    if (ipw.isField()) {
      const NamedDecl* FieldArg = ipw.getFieldND();
      assert(FieldArg && "field nullptr");
      IpSpace::setIpMap(State, Param, FieldArg);
    } else if (ipw.isObj()) {
      const NamedDecl* ObjArg = ipw.getObjND();
      assert(ObjArg && "obj nullptr");
      IpSpace::setIpMap(State, Param, ObjArg);
    }
  }
  addStateTransition(State, C, true);
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
    ProgramStateRef State = C.getState();
    bool stateChanged = false;
    const NamedDecl* ObjND = IpSpace::getRealND(State, ObjVD);
    DBG("obj log" << ObjND->getNameAsString())
    auto SI = StateInfo(C, State, BReporter, nullptr, ObjND, nullptr);
    TxSpace::checkTx(SI);
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

  if (FieldVD) {
    const NamedDecl* ObjND = IpSpace::getRealND(State, ObjVD);
    const NamedDecl* FieldND = IpSpace::getRealND(State, FieldVD);
    DBG("write field:" << FieldND->getNameAsString()
                       << "obj:" << ObjND->getNameAsString())
    auto SI = StateInfo(C, State, BReporter, nullptr, ObjND, FieldND);
    TxSpace::checkTx(SI);
    WriteSpace::logField(SI);
    stateChanged |= SI.stateChanged;
  } else if (ObjVD) {
    const NamedDecl* ObjND = IpSpace::getRealND(State, ObjVD);
    DBG("obj log" << ObjND->getNameAsString())
    auto SI = StateInfo(C, State, BReporter, nullptr, ObjND, nullptr);
    TxSpace::checkTx(SI);
    WriteSpace::logObj(SI);
    stateChanged |= SI.stateChanged;
  }

  addStateTransition(State, C, stateChanged);
}

void TxpChecker::handleTxBegin(const CallEvent& Call, CheckerContext& C) const {
  DBG("handleTxBegin")
  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  auto SI = StateInfo(C, State, BReporter, nullptr, nullptr, nullptr);
  TxSpace::begTx(SI);
  stateChanged |= SI.stateChanged;

  addStateTransition(State, C, stateChanged);
}

void TxpChecker::handleTxEnd(const CallEvent& Call, CheckerContext& C) const {
  DBG("handleTxEnd")
  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  auto SI = StateInfo(C, State, BReporter, nullptr, nullptr, nullptr);
  TxSpace::endTx(SI);
  stateChanged |= SI.stateChanged;

  addStateTransition(State, C, stateChanged);
}

void TxpChecker::checkBranchCondition(const Stmt* Cond,
                                      CheckerContext& C) const {
  // for avoiding within loop bugs
  /*
  const SourceManager& SM = C.getSourceManager();
  const Stmt* PS = getParentStmt(Cond, C);
  unsigned lineNo = 0; // invalid value
  if (const WhileStmt* WS = dyn_cast<WhileStmt>(PS)) {
    SourceLocation SL = WS->getLocEnd();
    unsigned lineNo = getSrcLineNo(SM, SL);
  } else if (const ForStmt* FS = dyn_cast<ForStmt>(PS)) {
    //SourceLocation SL = FS->getLocEnd();
    unsigned lineNo = getSrcLineNo(SM, SL);
  } else if (const DoStmt* DS = dyn_cast<DoStmt>(PS)) {
    SourceLocation SL = DS->getLocEnd();
    unsigned lineNo = getSrcLineNo(SM, SL);
  }

  // register loop
  if (lineNo) {
  }
  */
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
                                                    "Checks pmdk", "");
}
