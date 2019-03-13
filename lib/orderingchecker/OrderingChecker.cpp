//===-- OrderingChecker.cpp -----------------------------------------*
#include "OrderingChecker.h"

namespace clang::ento::nvm {

OrderingChecker::OrderingChecker() {}

void OrderingChecker::checkBeginFunction(CheckerContext &C) const {
  const StackFrameContext *sfc = C.getStackFrame();
  (void)sfc;
}

void OrderingChecker::checkEndAnalysis(ExplodedGraph &G, BugReporter &BR,
                                       ExprEngine &Eng) const {
  llvm::errs() << "End of analysis\n";
}

void OrderingChecker::checkBind(SVal Loc, SVal Val, const Stmt *S,
                                CheckerContext &C) const {
  llvm::outs() << "bind\n";
  Loc.dump();

  const TypedValueRegion* TR =
    dyn_cast_or_null<TypedValueRegion>(Loc.getAsRegion());

  if (!TR)
    return;

  llvm::outs() << "\n";
  QualType valTy = TR->getValueType();
  valTy.dump();

  llvm::outs() << "\n";
  Val.dump();
  llvm::outs() << "\n";
  S->dump();
  llvm::outs() << "\n";
}

void OrderingChecker::checkLocation(SVal Loc, bool IsLoad, const Stmt *S,
                                    CheckerContext &) const {
  llvm::outs() << "location\n";
}

void OrderingChecker::checkBranchCondition(const Stmt *Condition,
                                           CheckerContext &C) const {
  llvm::outs() << "checkBranchCondition\n";
  Condition->dump();
}

void OrderingChecker::checkPreCall(const CallEvent &Call,
                                   CheckerContext &C) const {
  llvm::outs() << "checkPreCall\n";
  Call.dump();
}


} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry &registry) {
  registry.addChecker<clang::ento::nvm::OrderingChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
