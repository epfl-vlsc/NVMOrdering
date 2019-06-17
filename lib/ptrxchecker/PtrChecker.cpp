//===-- PtrChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "PtrChecker.h"

namespace clang::ento::nvm {

void PtrChecker::checkASTDecl(const TranslationUnitDecl* CTUD,
                              AnalysisManager& Mgr, BugReporter& BR) const {
  TranslationUnitDecl* TUD = (TranslationUnitDecl*)CTUD;

  ASTContext& astContext = Mgr.getASTContext();
  PtrWalker ptrWalker(ptrFncs, ptrVars, astContext);
  ptrWalker.TraverseDecl(TUD);
  ptrWalker.finalize();

  printMsg("functions");
  ptrFncs.dump();
  printMsg("vars");
  ptrVars.dump();
}

void PtrChecker::checkBeginFunction(CheckerContext& C) const {
  bool isSkipFnc = ptrFncs.isSkip(C);
  bool isTopFnc = isTopFunction(C);

  // skip
  if (isSkipFnc && isTopFnc) {
    endExploration(C);
    return;
  }

  DBG("function: " << getFunctionDeclName(C))

  // make params dirty
  ProgramStateRef State = C.getState();
  const FunctionDecl* FD = getFuncDecl(C);
  const LocationContext* LC = C.getLocationContext();
  if (FD && !isTopFnc) {
    return;
  }

  for (unsigned idx = 0, e = FD->getNumParams(); idx != e; ++idx) {
    const ParmVarDecl* Param = FD->getParamDecl(idx);
    SVal Loc = State->getSVal(State->getRegion(Param, LC));
    DBGL(Loc, "paramloc")
    const MemRegion* MRParam = getTopBaseMemRegUnsafe(Loc);
    if (MRParam) {
      DBGR(MRParam, "param")
      ProgramStateRef NewState = Transitions::writePtr(State, MRParam);
      addNewState(State, NewState, C);
    }
  }
}

void PtrChecker::checkEndFunction(const ReturnStmt* RS,
                                  CheckerContext& C) const {}

void PtrChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                           CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  DBGS(S, "bind")
  DBGL(Loc, "bindloc")
  DBGL(Val, "bindval")

  AllocWalker aw;
  aw.TraverseStmt((Stmt*)S);

  // check tainted
  if (const FieldDecl* FD = getMemFieldDecl(Loc); ptrVars.isUsedVar(FD)) {
    const MemRegion* MRRead = getTopBaseMemRegUnsafe(Val);
    if (MRRead && Transitions::isPtrWritten(State, MRRead)) {
      DBGR(MRRead, "bindread")

      if (ExplodedNode* EN = C.generateErrorNode()) {
        DBG("report error")
        BugReportData BRData{nullptr, State,         C,
                             EN,      "not flushed", BReporter.NotFlushBug};
        BReporter.report(BRData);
      }
    }
  }

  // taint written value
  const MemRegion* MRWrite = getTopBaseMemRegLOrR(Loc, Val, !aw.isAllocUsed());
  if (MRWrite) {
    DBGR(MRWrite, "bindwrite")
    ProgramStateRef NewState = Transitions::writePtr(State, MRWrite);
    addNewState(State, NewState, C);
  }
}

void PtrChecker::checkPreCall(const CallEvent& Call, CheckerContext& C) const {
  const FunctionDecl* FD = getFuncDecl(Call);

  if (ptrFncs.isFlushFenceFnc(FD)) {
    handleFlushFenceFnc(Call, C);
  }
}

void PtrChecker::handleFlushFenceFnc(const CallEvent& Call,
                                     CheckerContext& C) const {
  if (Call.getNumArgs() > 3) {
    llvm::report_fatal_error("check flush function");
    return;
  }

  ProgramStateRef State = C.getState();
  SVal Loc = Call.getArgSVal(0);
  DBGS(Call.getOriginExpr(), "flush")
  DBGL(Loc, "floc")

  const MemRegion* MRFlush = getTopBaseMemRegUnsafe(Loc);
  if (MRFlush) {
    DBGR(MRFlush, "flushmr")

    ProgramStateRef NewState = Transitions::flushPtr(State, MRFlush);
    addNewState(State, NewState, C);
  }
}

bool PtrChecker::evalCall(const CallExpr* CE, CheckerContext& C) const {
  // skip evaluation of all log functions
  const FunctionDecl* FD = C.getCalleeDecl(CE);
  if (!FD) {
    return false;
  }

  return ptrFncs.isSkip(FD);
}

void PtrChecker::checkBranchCondition(const Stmt* S, CheckerContext& C) const {
  std::vector<const char*> names{"use_nvm"};
  printStmt(S,C,"b",true);
  if (usesNames(S, names)) {
    ProgramStateRef State = C.getState();
    const LocationContext* LC = C.getLocationContext();
    SVal Val = State->getSVal(S, LC);
    Optional<DefinedOrUnknownSVal> DVal = Val.getAs<DefinedOrUnknownSVal>();

    if (!DVal) {
      DBG("undef");
      endExploration(C);
      return;
    } 
    
    if (ProgramStateRef NewState = State->assume(*DVal, true)) {
      State = NewState;
      C.addTransition(NewState);
    }else{
      endExploration(C);
      return;
    }
  }
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::PtrChecker>(
      CHECKER_PLUGIN_NAME, "Checks persistent pointer usage", "");
}
