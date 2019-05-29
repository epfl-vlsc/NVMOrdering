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
  // DBGS(S, "bs")
  // DBGL(Loc, "bl")

  printStmt(S, C, "bs", true);
  printLoc(Val, "bval");
  printLoc(Loc, "bloc");
  
  if (auto ls = Val.getAs<nonloc::ConcreteInt>()) {
    auto l = ls.getValue();
    auto val = l.getValue();
    auto i = val.getExtValue();
    llvm::errs()<< i << "\n";
  }
}

void ExpChecker::checkPostCall(const CallEvent& Call, CheckerContext& C) const {
  const FunctionDecl* FD = getFuncDecl(Call);
  printND(FD, "fnc");
}

void ExpChecker::checkPreCall(const CallEvent& Call, CheckerContext& C) const {}

void ExpChecker::checkBranchCondition(const Stmt* S, CheckerContext& C) const {
  /*
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
    if (NewState == State) {
      DBG("fail")
    }
    C.addTransition(NewState);
    //SVal Valx = NewState->getSVal(S, LC);
    //DBGL(Valx, "val")
  }
  */
}

bool ExpChecker::evalCall(const CallExpr* CE, CheckerContext& C) const {
  static const size_t MEMCPY_ARGS = 3;
  const FunctionDecl* FD = C.getCalleeDecl(CE);
  if (!FD) {
    return false;
  }
  StringRef fncName = FD->getName();
  if (!fncName.equals("memcpy")) {
    return false;
  }

  if (CE->getNumArgs() != MEMCPY_ARGS) {
    llvm::report_fatal_error("what's wrong with memcpy");
  }

  ProgramStateRef State = C.getState();
  const LocationContext* LC = C.getLocationContext();

  const Expr* Arg0 = CE->getArg(0);
  const Expr* Arg1 = CE->getArg(1);
  SVal Loc = State->getSVal(Arg0, LC);
  SVal Val = State->getSVal(Arg1, LC);

  printStmt(CE, C, "es", true);
  printLoc(Loc, "eloc");
  printLoc(Val, "eval");
  
  const MemRegion* mloc = Loc.getAsRegion();
  const MemRegion* mval = Val.getAsRegion();

  printReg(mloc, "emloc");
  printReg(mval, "emval");

  SVal sloc = State->getSVal(mloc);
  SVal sval = State->getSVal(mval);

  printLoc(sloc, "esloc");
  printLoc(sval, "esval");

  State = State->bindLoc(sloc, sval, LC);
  
  C.addTransition(State);

  return true;
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::ExpChecker>(CHECKER_PLUGIN_NAME,
                                                    "Exp checker", "");
}
