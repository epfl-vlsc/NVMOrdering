//===-- MainChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "MainChecker.h"

namespace clang::ento::nvm {

void MainChecker::checkASTDecl(const TranslationUnitDecl* CTUD,
                                AnalysisManager& Mgr, BugReporter& BR) const {
  /*
  TranslationUnitDecl* TUD = (TranslationUnitDecl*)CTUD;
  // fill data structures
  const ASTContext& ASTC = Mgr.getASTContext();
  WriteWalker tudWalker(orderVars, orderFncs, ASTC);
  tudWalker.TraverseDecl(TUD);
  tudWalker.createUsedVars();

  orderVars.dump();
  orderFncs.dump();
  */
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
  // S->dump();
  /*
  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  if (const ValueDecl* VD = getValueDecl(Loc); VD) {
    if (orderVars.isUsedVar(VD)) {
      DBG("write " << VD->getNameAsString())
      auto& infoList = orderVars.getInfoList(VD);
      for (auto& BI : infoList) {
        auto SI = StateInfo(C, State, BReporter, &Loc, S, (const char*)VD);
        BI->write(SI);
        stateChanged |= SI.stateChanged;
      }
    }
  }

  addStateTransition(State, C, stateChanged);
  */
}

void MainChecker::checkPreCall(const CallEvent& Call,
                                CheckerContext& C) const {
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

void MainChecker::handleFenceFlush(const CallEvent& Call, CheckerContext& C) const {
  DBG("handleFlush")

  /*
  if (Call.getNumArgs() > 2) {
    llvm::report_fatal_error("check flush function");
    return;
  }

  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  SVal Loc = Call.getArgSVal(0);
  if (const ValueDecl* VD = getValueDecl(Loc); VD) {
    if (orderVars.isUsedVar(VD)) {
      DBG("flush " << VD->getNameAsString())
      auto& infoList = orderVars.getInfoList(VD);
      for (auto& BI : infoList) {
        auto SI =
            StateInfo(C, State, BReporter, &Loc, nullptr, (const char*)VD);
        BI->flush(SI);
        stateChanged |= SI.stateChanged;
      }
    }
  }

  addStateTransition(State, C, stateChanged);
  */
}

void MainChecker::checkBranchCondition(const Stmt* S,
                                        CheckerContext& C) const {
  
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
