//===-- MainChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "MainChecker.h"

namespace clang::ento::nvm {

void MainChecker::checkASTDecl(const TranslationUnitDecl* CTUD,
                               AnalysisManager& Mgr, BugReporter& BR) const {}

void MainChecker::checkBeginFunction(CheckerContext& C) const {
  DBG("checkBeginFunction")
}

void MainChecker::checkEndFunction(CheckerContext& C) const {}

void MainChecker::checkPreCall(const CallEvent& Call, CheckerContext& C) const {

}

void MainChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                            CheckerContext& C) const {}

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
