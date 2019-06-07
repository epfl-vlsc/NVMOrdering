//===-- PairChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "PairChecker.h"

namespace clang::ento::nvm {

void PairChecker::checkASTDecl(const TranslationUnitDecl* CTUD,
                                 AnalysisManager& Mgr, BugReporter& BR) const {
  /*
  TranslationUnitDecl* TUD = (TranslationUnitDecl*)CTUD;

  // find vars and functions to track and analyze
  MainVars& mainVars = lattice.getMainVars();
  MainFncs& mainFncs = lattice.getMainFncs();
  ASTContext& astContext = Mgr.getASTContext();
  MainWalker mainWalker(mainVars, mainFncs, astContext);
  mainWalker.TraverseDecl(TUD);
  mainWalker.finalize();

  lattice.dump();
  */

 /*
  // run data flow analysis and report errors
  const FunctionDecl* FD = dyn_cast<FunctionDecl>(D);
  if (!FD || !analyzer.isAnalyzedFunction(FD)) {
    return;
  }

  // analyze single function inter-procedurally
  DataFlow dataflow(FD, lattice, mgr, BR);
  dataflow.computeDataFlow();
  dataflow.dump();
  dataflow.reportBugs();
  */
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::PairChecker>(
      CHECKER_PLUGIN_NAME, "Data flow pair checker", "");
}
