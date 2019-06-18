//===-- PairChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "LogChecker.h"

namespace clang::ento::nvm {

void LogChecker::checkEndOfTranslationUnit(const TranslationUnitDecl* CTUD,
                                            AnalysisManager& Mgr,
                                            BugReporter& BR) const {
  LogAnalyzer analyzer;
  TranslationUnitDecl* TUD = (TranslationUnitDecl*)CTUD;
  analyzer.analyzeTUD(TUD, Mgr, BR, this);
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::LogChecker>(
      CHECKER_PLUGIN_NAME, "Data flow log checker", "");
}
