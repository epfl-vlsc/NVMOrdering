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
  }
}

void LogChecker::handleLog(const CallEvent& Call, CheckerContext& C) const {
  DBG("handleLog")

  if (Call.getNumArgs() > 2) {
    llvm::report_fatal_error("check log function");
    return;
  }

  SVal Loc = Call.getArgSVal(0);
  const MemRegion* Region = Loc.getAsRegion();
  if (const NamedDecl* ND = getLoggedField(Region); ND) {
    if (logVars.isUsedVar(ND)) {
      DBG("log " << ND->getNameAsString())
      ProgramStateRef State = C.getState();
      auto SI = StateInfo(C, State, BReporter, &Loc, nullptr, ND);
      LogSpace::logData(SI);
    }
  } 
}

bool LogChecker::evalCall(const CallExpr* CE, CheckerContext& C) const {
  // skip evaluation of all log functions
  const FunctionDecl* FD = C.getCalleeDecl(CE);
  if (!FD) {
    return false;
  }

  if (logFncs.isLogFunction(FD)) {
    return true;
  }

  return false;
}


void LogChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                           CheckerContext& C) const {
  DBG("checkBind")

  const MemRegion* Region = Loc.getAsRegion();
  if(!Region){
    return;
  }

  if (const NamedDecl* ND = getValueDecl(Region); ND) {
    if (logVars.isUsedVar(ND)) {
      DBG("write " << ND->getNameAsString())
      ProgramStateRef State = C.getState();
      auto SI = StateInfo(C, State, BReporter, &Loc, S, ND);
      LogSpace::writeData(SI);
    }
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
                                                    "Checks logging use");
}
