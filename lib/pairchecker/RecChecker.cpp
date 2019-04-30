//===-- RecChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "RecChecker.h"

namespace clang::ento::nvm {

void RecChecker::checkASTDecl(const TranslationUnitDecl* CTUD,
                               AnalysisManager& Mgr, BugReporter& BR) const {}

void RecChecker::checkBeginFunction(CheckerContext& C) const {
  DBG("checkBeginFunction")
}

void RecChecker::checkEndFunction(CheckerContext& C) const {}

void RecChecker::checkPreCall(const CallEvent& Call, CheckerContext& C) const {

}

void RecChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                            CheckerContext& C) const {}

void RecChecker::checkBranchCondition(const Stmt* S, CheckerContext& C) const {

}

void RecChecker::addStateTransition(ProgramStateRef& State, CheckerContext& C,
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
  registry.addChecker<clang::ento::nvm::RecChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
