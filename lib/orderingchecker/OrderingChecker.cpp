//===-- OrderingChecker.cpp -----------------------------------------*
#include "OrderingChecker.h"

namespace clang::ento::nvm
{

OrderingChecker::OrderingChecker() {}

void OrderingChecker::checkBeginFunction(CheckerContext &C) const
{
  bool isAnnotated = nvmFncInfo.isAnnotatedFunction(C);
  bool isTopFnc = isTopFunction(C);

  //if not an annotated function, do not analyze
  if (!isAnnotated && isTopFnc)
  {
    ExplodedNode *ErrNode = C.generateErrorNode();
    if (!ErrNode)
      return;
  }
}

void OrderingChecker::checkEndAnalysis(ExplodedGraph &G, BugReporter &BR,
                                       ExprEngine &Eng) const
{
  //llvm::errs() << "End of analysis\n";
}

void OrderingChecker::checkBind(SVal Loc, SVal Val, const Stmt *S,
                                CheckerContext &C) const
{
  /*
  llvm::outs() << "bind\n";
  Loc.dump();

  const TypedValueRegion *TR =
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
  */
}

void OrderingChecker::checkBranchCondition(const Stmt *Condition,
                                           CheckerContext &C) const
{
  //llvm::outs() << "checkBranchCondition\n";
  //Condition->dump();
}

void OrderingChecker::checkPreCall(const CallEvent &Call,
                                   CheckerContext &C) const
{
  //llvm::outs() << "checkPreCall\n";
  //Call.dump();
}

void OrderingChecker::checkEndOfTranslationUnit(const TranslationUnitDecl *TU,
                                                AnalysisManager &Mgr,
                                                BugReporter &BR) const
{
  //llvm::outs() << "checkEndOfTranslationUnit\n";
}

void OrderingChecker::checkASTDecl(const FunctionDecl *D, AnalysisManager &Mgr,
                                   BugReporter &BR) const
{
  nvmFncInfo.analyzeIfAnnotated(D);
}

void OrderingChecker::checkASTDecl(const RecordDecl *D, AnalysisManager &Mgr, BugReporter &BR) const
{
  llvm::outs() << D->getName() << "\n";
  for (RecordDecl::field_iterator I = D->field_begin(), E = D->field_end();
       I != E; ++I)
  {
    llvm::outs() << I->getName() << " " << I->getType().getAsString() << "\n";
  }
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry &registry)
{
  registry.addChecker<clang::ento::nvm::OrderingChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
