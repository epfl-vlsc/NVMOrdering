//===-- DfMainChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "DfMainChecker.h"

namespace clang::ento::nvm {

void DfMainChecker::checkASTCodeBody(const Decl* D, AnalysisManager& mgr,
                                     BugReporter& BR) const {
  // run data flow analysis and report errors
  const FunctionDecl* FD = dyn_cast<FunctionDecl>(D);
  if (!FD || !lattice.isAnalyze(FD)) {
    return;
  }

  // analyze single function inter-procedurally
  DataFlow dataflow(FD, lattice, mgr);
  dataflow.computeDataFlow();
  dataflow.dump();
}

void DfMainChecker::checkASTDecl(const TranslationUnitDecl* CTUD,
                                 AnalysisManager& Mgr, BugReporter& BR) const {
  TranslationUnitDecl* TUD = (TranslationUnitDecl*)CTUD;

  // find vars and functions to track and analyze
  MainVars& mainVars = lattice.getMainVars();
  MainFncs& mainFncs = lattice.getMainFncs();
  ASTContext& astContext = Mgr.getASTContext();
  MainWalker mainWalker(mainVars, mainFncs, astContext);
  mainWalker.TraverseDecl(TUD);
  mainWalker.finalize();

  lattice.dump();
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::DfMainChecker>(
      CHECKER_PLUGIN_NAME, "Data flow main checker", "");
}
