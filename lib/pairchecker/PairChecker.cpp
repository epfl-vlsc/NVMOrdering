//===-- PairChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "PairChecker.h"

namespace clang::ento::nvm {

void PairChecker::checkASTDecl(const TranslationUnitDecl* CTUD,
                               AnalysisManager& Mgr, BugReporter& BR) const {

  PairAnalyzer analyzer;
  TranslationUnitDecl* TUD = (TranslationUnitDecl*)CTUD;
  analyzer.analyzeTUD(TUD, Mgr, BR, this);
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::PairChecker>(
      CHECKER_PLUGIN_NAME, "Data flow pair checker", "");
}
