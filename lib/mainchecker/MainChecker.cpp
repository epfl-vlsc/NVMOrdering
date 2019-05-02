//===-- MainChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "MainChecker.h"

namespace clang::ento::nvm {

void MainChecker::checkASTDecl(const TranslationUnitDecl* CTUD,
                               AnalysisManager& Mgr, BugReporter& BR) const {

  TranslationUnitDecl* TUD = (TranslationUnitDecl*)CTUD;
  // fill data structures

  const ASTContext& astContext = Mgr.getASTContext();
  MainWalker mainWalker(mainVars, mainFncs, astContext);
  mainWalker.TraverseDecl(TUD);
  mainWalker.finalize();

  mainVars.dump();
  mainFncs.dump();
}

void MainChecker::checkBeginFunction(CheckerContext& C) const {

  DBG("checkBeginFunction")
  bool isAnnotated = mainFncs.isPersistentFunction(C);
  bool isTopFnc = isTopFunction(C);

  // if not an annotated function, do not analyze
  if (!isAnnotated && isTopFnc) {
    handleEnd(C);
  }
}

void MainChecker::handleEnd(CheckerContext& C) const {
  ExplodedNode* ErrNode = C.generateErrorNode();
  if (!ErrNode)
    return;
}

void MainChecker::checkEndFunction(CheckerContext& C) const {
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

void MainChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                            CheckerContext& C) const {
  
  /*
  DBG("checkBind")
  ProgramStateRef State = C.getState();
  bool stateChanged = false;
  bool isCheck = false;

  if (const NamedDecl* ND = getValueDecl(Loc); ND) {
    const NamedDecl* RD = getRDFromFD(ND);

    if (mainVars.isUsedVar(ND)) {
      DBG("write " << ND->getNameAsString())
      auto& pairList = mainVars.getPairList(ND);
      for (auto& PI : pairList) {
        auto SI = StateInfo(C, State, BReporter, &Loc, S, PI);
        if (PI->isData(ND)) {
          WriteSpace::writeData(SI);
        } else {
          isCheck = true;
          WriteSpace::writeCheck(SI);
        }
        stateChanged |= SI.stateChanged;
      }
    }

    if (mainVars.isUsedVar(RD) && !isCheck) {
      DBG("write " << RD->getNameAsString())
      auto& pairList = mainVars.getPairList(RD);
      for (auto& PI : pairList) {
        auto SI = StateInfo(C, State, BReporter, &Loc, S, PI);
        if (PI->isData(RD)) {
          WriteSpace::writeData(SI);
        } else {
          WriteSpace::writeCheck(SI);
        }
        stateChanged |= SI.stateChanged;
      }
    }
  }

  addStateTransition(State, C, stateChanged);
  */
}

void MainChecker::checkPreCall(const CallEvent& Call, CheckerContext& C) const {
  const FunctionDecl* FD = getFuncDecl(Call);

  if (mainFncs.isFlushFenceFunction(FD)) {
    handleFenceFlush(Call, C);
  }
}

bool MainChecker::evalCall(const CallExpr* CE, CheckerContext& C) const {
  // skip evaluation of all log functions
  const FunctionDecl* FD = C.getCalleeDecl(CE);
  if (!FD) {
    return false;
  }

  if (mainFncs.isFlushFenceFunction(FD)) {
    return true;
  }

  return false;
}

void MainChecker::handleFenceFlush(const CallEvent& Call,
                                   CheckerContext& C) const {
  
  /*
  DBG("handleFlush")
  if (Call.getNumArgs() > 2) {
    llvm::report_fatal_error("check flush function");
    return;
  }

  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  SVal Loc = Call.getArgSVal(0);
  const MemRegion* Region = Loc.getAsRegion();

  if (const NamedDecl* ND = getFlushND(Region); ND) {
    if (mainVars.isUsedVar(ND)) {
      DBG("flush " << ND->getNameAsString())
      auto& pairList = mainVars.getPairList(ND);
      for (auto& PI : pairList) {
        auto SI = StateInfo(C, State, BReporter, &Loc, nullptr, PI);
        if (PI->isData(ND)) {
          WriteSpace::flushFenceData(SI);
        } else {
          WriteSpace::flushFenceCheck(SI);
        }
        stateChanged |= SI.stateChanged;
      }
    }
  }

  addStateTransition(State, C, stateChanged);
  */
}

void MainChecker::checkBranchCondition(const Stmt* S, CheckerContext& C) const {

}

void MainChecker::addStateTransition(ProgramStateRef& State, CheckerContext& C,
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
  registry.addChecker<clang::ento::nvm::MainChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
