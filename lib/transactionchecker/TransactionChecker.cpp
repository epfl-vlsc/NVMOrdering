//===-- OrderingChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "TransactionChecker.h"

namespace clang::ento::nvm {

void checkBeginFunction(CheckerContext& Ctx) const{
  //if analysis takes time, consider implementing keyword specific unc
}

  void checkBind(SVal Loc, SVal Val, const Stmt *S, CheckerContext &) const;

  void checkASTDecl(const FunctionDecl* D, AnalysisManager& Mgr,
                    BugReporter& BR) const;

  void checkASTDecl(const DeclaratorDecl* D, AnalysisManager& Mgr,
                    BugReporter& BR) const;

  void checkDeadSymbols(SymbolReaper& SymReaper, CheckerContext& C) const;

  ProgramStateRef checkPointerEscape(ProgramStateRef State,
                                     const InvalidatedSymbols& Escaped,
                                     const CallEvent* Call,
                                     PointerEscapeKind Kind) const;

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::TransactionChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
