//===-- ExpChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "ExpChecker.h"

namespace clang::ento::nvm {

void ExpChecker::checkASTDecl(const RecordDecl* RD, AnalysisManager& Mgr,
                              BugReporter& BR) const {}

void ExpChecker::checkASTDecl(const TranslationUnitDecl* CTUD,
                              AnalysisManager& Mgr, BugReporter& BR) const {}

void ExpChecker::checkBeginFunction(CheckerContext& C) const {
  const LocationContext* LC = C.getLocationContext();
  const Decl* BD = LC->getDecl();
}

void ExpChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                           CheckerContext& C) const {
  DBGS(S, "bs")
  DBGL(Loc, "bl")
}

void ExpChecker::checkPostCall(const CallEvent& Call, CheckerContext& C) const {
  const FunctionDecl* FD = getFuncDecl(Call);
  const Stmt* E = Call.getOriginExpr();
}

void ExpChecker::checkPreCall(const CallEvent& Call, CheckerContext& C) const {}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::ExpChecker>(CHECKER_PLUGIN_NAME,
                                                    "Exp checker", "");
}
