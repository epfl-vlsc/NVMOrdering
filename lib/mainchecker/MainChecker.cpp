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
  DBG("checkBind")
  ProgramStateRef State = C.getState();
  bool stateChanged = false;
  bool isCheck = false;

  if (const FieldDecl* FD = getFDFromLoc(Loc); FD) {
    HandleInfo HI{State, C, S, stateChanged, isCheck};
    handleWrite(FD, HI);
    if (!isCheck) {
      const RecordDecl* RD = FD->getParent();
      handleWrite(RD, HI);
    }
  }

  addStateTransition(State, S, C, stateChanged);
}

void MainChecker::handleWrite(const NamedDecl* ND, HandleInfo& HI) const {
  auto& [State, C, S, stateChanged, isCheck] = HI;
  if (mainVars.isUsedVar(ND)) {
    DBG("write " << ND->getNameAsString())
    auto& pairList = mainVars.getPairList(ND);
    for (auto& PI : pairList) {
      auto SI = StateInfo(C, State, BReporter, S, PI, ND);
      Transitions::write(SI);
      stateChanged |= SI.stateChanged;
      isCheck |= SI.isCheck();
    }
  }
}

void MainChecker::checkPreCall(const CallEvent& Call, CheckerContext& C) const {
  const FunctionDecl* FD = getFuncDecl(Call);

  if (mainFncs.isFlushFenceFnc(FD)) {
    handleFlushFnc<true>(Call, C);
  } else if (mainFncs.isFlushOptFnc(FD)) {
    handleFlushFnc<false>(Call, C);
  } else if (mainFncs.isVfenceFnc(FD)) {
    handleFence<false>(Call, C);
  } else if (mainFncs.isPfenceFnc(FD)) {
    handleFence<true>(Call, C);
  }
}

bool MainChecker::evalCall(const CallExpr* CE, CheckerContext& C) const {
  // skip evaluation of all log functions
  const FunctionDecl* FD = C.getCalleeDecl(CE);
  if (!FD) {
    return false;
  }

  if (mainFncs.isUsedFnc(FD)) {
    return true;
  }

  return false;
}

template <bool pfence>
void MainChecker::handleFence(const CallEvent& Call, CheckerContext& C) const {
  DBG("handlePfence")
  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  const Expr* E = Call.getOriginExpr();

  for (auto& [PI, _] : State->get<WriteMap>()) {
    auto SI = StateInfo(C, State, BReporter, E, PI, nullptr);

    if constexpr (pfence == true)
      Transitions::pfence(SI);
    else
      Transitions::vfence(SI);

    stateChanged |= SI.stateChanged;
  }

  addStateTransition(State, E, C, stateChanged);
}

template <bool fence>
void MainChecker::handleFlushFnc(const CallEvent& Call,
                                 CheckerContext& C) const {
  DBG("handleFlush")
  if (Call.getNumArgs() > 2) {
    llvm::report_fatal_error("check flush function");
    return;
  }

  ProgramStateRef State = C.getState();
  bool stateChanged = false;
  bool _ = false;

  SVal Loc = Call.getArgSVal(0);
  const Expr* E = Call.getOriginExpr();

  HandleInfo HI{State, C, E, stateChanged, _};
  if (const FieldDecl* FD = getFDFromLoc(Loc); FD) {
    handleFlush<fence>(FD, HI);
  } else if (const RecordDecl* RD = getRDFromRecordLoc(Loc); RD) {
    handleFlush<fence>(RD, HI);

    for (const FieldDecl* FD : RD->fields()) {
      if (mainVars.isUsedVar(FD)) {
        auto& pairList = mainVars.getPairList(FD);
        for (auto& PI : pairList) {
          if (!PI->isData(RD) || !PI->isCheck(FD))
            handleFlush<fence>(FD, HI);
        }
      }
    }
  }

  addStateTransition(State, E, C, stateChanged);
}

template <bool fence>
void MainChecker::handleFlush(const NamedDecl* ND, HandleInfo& HI) const {
  auto& [State, C, E, stateChanged, isData] = HI;
  if (mainVars.isUsedVar(ND)) {
    DBG("flush " << ND->getNameAsString())
    auto& pairList = mainVars.getPairList(ND);
    for (auto& PI : pairList) {
      auto SI = StateInfo(C, State, BReporter, E, PI, ND);

      if constexpr (fence == true)
        Transitions::flushFence(SI);
      else
        Transitions::flush(SI);

      stateChanged |= SI.stateChanged;
    }
  }
}

void MainChecker::checkBranchCondition(const Stmt* S, CheckerContext& C) const {

}

void MainChecker::addStateTransition(ProgramStateRef& State, const Stmt* S,
                                     CheckerContext& C,
                                     bool stateChanged) const {
  if (stateChanged) {
    DBG("state transition")
    if(S){
      SourceRange SR = S->getSourceRange();
      SlSpace::saveSR(State, SR);
    }
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