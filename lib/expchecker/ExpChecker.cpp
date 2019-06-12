//===-- ExpChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "ExpChecker.h"

namespace clang::ento::nvm {

void ExpChecker::checkEndOfTranslationUnit(const TranslationUnitDecl* TU,
                                           AnalysisManager& Mgr,
                                           BugReporter& BR) const {
  ASTContext& ACtx = Mgr.getASTContext();
  const TranslationUnitDecl* TUD = ACtx.getTranslationUnitDecl();
  SourceManager& SM = ACtx.getSourceManager();
  AnalysisDeclContext* ADC = Mgr.getAnalysisDeclContext(TUD);

  std::unique_ptr<BugType> BT_Exact;
  if (!BT_Exact)
    BT_Exact.reset(new BugType(this, "Exact code clone", "Code clone"));

  auto L = PathDiagnosticLocation::createBegin(TUD, SM, ADC);
  auto R =
      llvm::make_unique<BugReport>(*BT_Exact, "Duplicate code detected", L);

  BR.emitReport(std::move(R));
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::ExpChecker>(CHECKER_PLUGIN_NAME,
                                                    "Exp checker", "");
}
