//===-- ExpChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "ExpChecker.h"

namespace clang::ento::nvm {

void ExpChecker::checkASTDecl(const RecordDecl* RD, AnalysisManager& Mgr,
                              BugReporter& BR) const {}

void ExpChecker::checkASTDecl(const TranslationUnitDecl* CTUD,
                              AnalysisManager& Mgr, BugReporter& BR) const {}

void ExpChecker::checkBeginFunction(CheckerContext& C) const {}

void ExpChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                           CheckerContext& C) const {
  DBGS(S, "bs")
  DBGL(Loc, "bl")
}

void ExpChecker::checkPostCall(const CallEvent& Call, CheckerContext& C) const {
  const FunctionDecl* FD = getFuncDecl(Call);
  DBGN(FD, "fd")
}

void ExpChecker::checkPreCall(const CallEvent& Call, CheckerContext& C) const {}

void ExpChecker::checkBranchCondition(const Stmt* S, CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  const LocationContext* LC = C.getLocationContext();
  SVal Val = State->getSVal(S, LC);
  Optional<DefinedOrUnknownSVal> DVal = Val.getAs<DefinedOrUnknownSVal>();

  DBGL(Val, "val")
  if (State->assume(*DVal, true)) {
    DBG("false")
  }
  if (State->assume(*DVal, false)) {
    DBG("false")
    ProgramStateRef NewState = C.getState();
    SValBuilder& SVB = C.getSValBuilder();
    SVal TrueValue = SVB.makeTruthVal(true);
    DBGL(TrueValue, "truthval")
    DBGS(S, "s")
    NewState = NewState->BindExpr(S, LC, TrueValue);
    if(NewState == State){
      DBG("fail")
    }
    C.addTransition(NewState);
    SVal Valx = NewState->getSVal(S, LC);
    DBGL(Valx, "val")
  }
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::ExpChecker>(CHECKER_PLUGIN_NAME,
                                                    "Exp checker", "");
}
