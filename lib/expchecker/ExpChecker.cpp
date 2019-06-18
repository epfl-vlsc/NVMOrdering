//===-- ExpChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "ExpChecker.h"
#include "ExpParser.h"

namespace clang::ento::nvm {

void ExpChecker::checkEndOfTranslationUnit(const TranslationUnitDecl* TUD,
                                           AnalysisManager& Mgr,
                                           BugReporter& BR) const {
  ExpParser parser((TranslationUnitDecl*)TUD);
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::ExpChecker>(CHECKER_PLUGIN_NAME,
                                                    "Exp checker", "");
}
