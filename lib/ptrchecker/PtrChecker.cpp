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
  /*
  bool isAnnotated = ptrFncs.isPersistentFunction(C);
  bool isTopFnc = isTopFunction(C);

  // if not an annotated function, do not analyze
  if (!isAnnotated && isTopFnc) {
    handleEnd(C);
  }
  */
}

void PtrChecker::handleEnd(CheckerContext& C) const {
  ExplodedNode* ErrNode = C.generateErrorNode();
  if (!ErrNode)
    return;
}

void PtrChecker::checkEndFunction(CheckerContext& C) const {
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
  DBG("checkBind")
  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  /*
  if (const FieldDecl* FD = getFDFromLoc(Loc); FD) {
    HandleInfo HI{State, C, S, stateChanged, isCheck};
    handleWrite(FD, HI);
    if (!isCheck) {
      const RecordDecl* RD = FD->getParent();
      handleWrite(RD, HI);
    }
  }
  */

  addStateTransition(State, S, C, stateChanged);
}

void PtrChecker::checkPreCall(const CallEvent& Call, CheckerContext& C) const {
  const FunctionDecl* FD = getFuncDecl(Call);

  if (ptrFncs.isFlushFenceFnc(FD)) {
    handleFlushFenceFnc(Call, C);
  }
}

void PtrChecker::handleFlushFenceFnc(const CallEvent& Call,
                                     CheckerContext& C) const {}

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

void PtrChecker::addStateTransition(ProgramStateRef& State, const Stmt* S,
                                    CheckerContext& C,
                                    bool stateChanged) const {
  if (stateChanged) {
    DBG("state transition")
    /*
    if(S){
      SourceRange SR = S->getSourceRange();
      SlSpace::saveSR(State, SR);
    }
    */
    C.addTransition(State);
  }
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::PtrChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
