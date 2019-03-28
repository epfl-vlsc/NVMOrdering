//===-- OrderingChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "ExpChecker.h"

namespace clang::ento::nvm {

void ExpChecker::checkPreCall(const CallEvent& Call, CheckerContext& C) const {
  /*
  if (auto* II = Call.getCalleeIdentifier()) {
    StringRef name = II->getName();
    llvm::outs() << name << "\n";
  }
  */
}

void ExpChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                           CheckerContext&) const {
  /*
  Loc.dump();
  llvm::outs() << "\n";
  */
}

void ExpChecker::checkBranchCondition(const Stmt* Condition,
                                      CheckerContext& C) const {
  /*
  ProgramStateRef State = C.getState();
  SVal Val = State->getSVal(Condition, C.getLocationContext());
  Optional<DefinedOrUnknownSVal> DVal = Val.getAs<DefinedOrUnknownSVal>();
  const ASTContext& AC = C.getASTContext();
  Condition->dumpPretty(AC);
  if(State->assume(*DVal, false)){
    llvm::outs() << " false\n";
  }else{
    llvm::outs() << " true\n";
  }
  */
}

void ExpChecker::checkPostCall(const CallEvent& Call, CheckerContext& C) const {
  if (auto* II = Call.getCalleeIdentifier()) {
    if (II->isStr("malloc")) {
      SVal RetVal = Call.getReturnValue();
      if (const SymbolicRegion* R =
              dyn_cast_or_null<SymbolicRegion>(RetVal.getAsRegion())) {
        SValBuilder& SVB = C.getSValBuilder();
        DefinedOrUnknownSVal Extent = R->getExtent(SVB);
        ProgramStateRef State = C.getState();
      }
    }
  }
}

void ExpChecker::checkASTDecl(const TranslationUnitDecl* D,
                              AnalysisManager& Mgr, BugReporter& BR) const {

  llvm::outs() << "lol\n";
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::ExpChecker>(CHECKER_PLUGIN_NAME,
                                                    "Exp checker");
}
