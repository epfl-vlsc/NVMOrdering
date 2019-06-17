//===-- LogChecker.cpp -----------------------------------------*

#include "LogChecker.h"

namespace clang::ento::nvm {

void LogChecker::checkASTDecl(const FunctionDecl* FD, AnalysisManager& Mgr,
                              BugReporter& BR) const {
  logFncs.insertIfKnown(FD);
}

void LogChecker::checkASTDecl(const FieldDecl* FD, AnalysisManager& Mgr,
                              BugReporter& BR) const {
  logVars.insertIfKnown(FD);
}

void LogChecker::checkBeginFunction(CheckerContext& C) const {
  DBG("checkBeginFunction")
  const FunctionDecl* FD = getFuncDecl(C);
  bool isAnnotated = logFncs.isAnalysisFunction(FD);
  bool isTopFnc = isTopFunction(C);

  // if not an annotated function, do not analyze
  if (!logFncs.checkTxMode()) {
    logFncs.useTxMode();
    logFncs.setTxMode();
  }

  if (!isAnnotated && isTopFnc) {
    handleEnd(C);
  }
}

void LogChecker::checkPostCall(const CallEvent& Call, CheckerContext& C) const {
  const FunctionDecl* FD = getFuncDecl(Call);
  if (!FD) {
    return;
  }

  if (logFncs.isLogFunction(FD)) {
    handleLog(Call, C);
  } else if (logFncs.isTxBegFunction(FD)) {
    handleTxBeg(Call, C);
  } else if (logFncs.isTxEndFunction(FD)) {
    handleTxEnd(Call, C);
  }
}

void LogChecker::handleLog(const CallEvent& Call, CheckerContext& C) const {
  DBG("handleLog")

  if (Call.getNumArgs() > 2) {
    llvm::report_fatal_error("check log function");
    return;
  }

  SVal Loc = Call.getArgSVal(0);
  const Expr* E = Call.getOriginExpr();

  printLoc(Loc, "log");
  if (const NamedDecl* ND = getMemLogFieldDecl(Loc); logVars.isUsedVar(ND)) {
    DBG("log " << ND->getNameAsString())
    ProgramStateRef State = C.getState();
    auto SI = StateInfo(C, State, BReporter, E, ND);
    if (logFncs.isTxEnabled()) {
      TxSpace::checkTx(SI);
    }
    LogSpace::logData(SI);
  }
}

void LogChecker::handleTxBeg(const CallEvent& Call, CheckerContext& C) const {
  DBG("handleTxBegin")
  ProgramStateRef State = C.getState();
  TxSpace::begTx(State, C);
}

void LogChecker::handleTxEnd(const CallEvent& Call, CheckerContext& C) const {
  DBG("handleTxEnd")
  ProgramStateRef State = C.getState();
  TxSpace::endTx(State, C);
}

bool LogChecker::evalCall(const CallExpr* CE, CheckerContext& C) const {
  // skip evaluation of all log functions
  const FunctionDecl* FD = C.getCalleeDecl(CE);
  if (!FD) {
    return false;
  }

  if (logFncs.isUsedFnc(FD)) {
    return true;
  }

  return false;
}

void LogChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                           CheckerContext& C) const {
  DBG("checkBind")

  printLoc(Loc, "loc");
  if (const NamedDecl* ND = getMemFieldDecl(Loc); logVars.isUsedVar(ND)) {
    DBG("write " << ND->getNameAsString())
    ProgramStateRef State = C.getState();
    auto SI = StateInfo(C, State, BReporter, S, ND);
    if (logFncs.isTxEnabled()) {
      TxSpace::checkTx(SI);
    }
    LogSpace::writeData(SI);
  }
}

void LogChecker::checkBranchCondition(const Stmt* S, CheckerContext& C) const {}

void LogChecker::handleEnd(CheckerContext& C) const {
  ExplodedNode* ErrNode = C.generateErrorNode();
  if (!ErrNode)
    return;
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::LogChecker>(CHECKER_PLUGIN_NAME,
                                                    "Checks logging use", "");
}
