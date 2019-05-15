//===-- PtrChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "PtrChecker.h"

namespace clang::ento::nvm {

void PtrChecker::checkASTDecl(const FunctionDecl* FD, AnalysisManager& Mgr,
                              BugReporter& BR) const {
  ptrFncs.insertIfKnown(FD);
}

void PtrChecker::checkASTDecl(const FieldDecl* FD, AnalysisManager& Mgr,
                              BugReporter& BR) const {
  ptrVars.insertIfKnown(FD);
}

void PtrChecker::checkBeginFunction(CheckerContext& C) const {
  DBG("checkBeginFunction")
  bool isAnnotated = ptrFncs.isPersistentFunction(C);
  bool isTopFnc = isTopFunction(C);

  // if not an annotated function, do not analyze
  if (!isAnnotated && isTopFnc) {
    handleEnd(C);
  }
}

void PtrChecker::handleEnd(CheckerContext& C) const {
  ExplodedNode* ErrNode = C.generateErrorNode();
  if (!ErrNode)
    return;
}

void PtrChecker::checkEndFunction(const ReturnStmt* RS,
                                  CheckerContext& C) const {
  DBG("checkEndFunction")
  /*
  bool isAnnotated = orderFncs.isPersistentFunction(C);
  bool isTopFnc = isTopFunction(C);
  if (isAnnotated && isTopFnc) {
    // ensured it is the top function and annotated
    ProgramStateRef State = C.getState();

    checkMapStates<CheckMap>(State, C);
    checkMapStates<DclMap>(State, C);
    checkMapStates<SclMap>(State, C);
  }
  */
}

void PtrChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                           CheckerContext& C) const {
  ProgramStateRef State = C.getState();

  DBGL(Loc, "bind")
  DBGS(S, "bind")

  // check tainted
  if (const FieldDecl* FD = getMemFieldDecl(Loc); ptrVars.inValues(FD)) {
    if (Transitions::isPtrWritten(State, Val)) {
      if (ExplodedNode* EN = C.generateErrorNode()) {
        DBG("generate error node")
        BugReportData BRData{nullptr, State,         C,
                             EN,      "not flushed", BReporter.NotFlushBug};
        BReporter.report(BRData);
      }
    }
  }

  // taint written value
  ProgramStateRef NewState = Transitions::writePtr(State, Val);
  if (NewState != State) {
    C.addTransition(NewState);
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
  if (Call.getNumArgs() > 2) {
    llvm::report_fatal_error("check flush function");
    return;
  }

  ProgramStateRef State = C.getState();
  SVal Loc = Call.getArgSVal(0);
  const Expr* E = Call.getOriginExpr();
  DBGL(Loc, "flush")
  DBGS(E, "flush")

  ProgramStateRef NewState = Transitions::flushPtr(State, Loc);
  if (NewState != State) {
    C.addTransition(NewState);
  }
}

bool PtrChecker::evalCall(const CallExpr* CE, CheckerContext& C) const {
  // skip evaluation of all log functions
  const FunctionDecl* FD = C.getCalleeDecl(CE);
  if (!FD) {
    return false;
  }

  if (ptrFncs.isUsedFnc(FD)) {
    return true;
  }

  return false;
}

void PtrChecker::checkBranchCondition(const Stmt* S, CheckerContext& C) const {}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::PtrChecker>(
      CHECKER_PLUGIN_NAME, "Checks persistent pointer usage", "");
}
