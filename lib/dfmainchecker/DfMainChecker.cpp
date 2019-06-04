//===-- DfMainChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "DfMainChecker.h"


namespace clang::ento::nvm {

void traverse(const CFG* cfg, AnalysisManager& mgr) {
  if (!cfg)
    return;

  for (CFG::const_iterator it = cfg->begin(), ei = cfg->end(); it != ei; ++it) {
    const CFGBlock* block = *it;
    block->dump();
    for (CFGBlock::const_iterator bi = block->begin(), be = block->end();
         bi != be; ++bi) {
      if (Optional<CFGStmt> CS = bi->getAs<CFGStmt>()) {
        const Stmt* S = CS->getStmt();
        printStmt(S, "s");
        if (const CallExpr* TheCall = dyn_cast<CallExpr>(S)) {
          // Get the callee.
          const FunctionDecl* FD = TheCall->getDirectCallee();

          if (!FD)
            continue;

          // Get the name of the callee. If it's a builtin, strip off the
          // prefix.
          IdentifierInfo* FnInfo = FD->getIdentifier();
          if (!FnInfo)
            continue;

          if (FnInfo->isStr("malloc") || FnInfo->isStr("_MALLOC")) {
          }
        }
      }
    }
  }
}

void DfMainChecker::checkASTCodeBody(const Decl* D, AnalysisManager& mgr,
                                     BugReporter& BR) const {
  //run data flow analysis and report errors
  const FunctionDecl* FD = dyn_cast<FunctionDecl>(D);
  if (!FD || mainFncs.isSkip(FD)) {
    return;
  }

  auto& trackSet = mainVars.getTrackSet(FD);
  DataFlow dataflow(FD, trackSet, mgr);
  dataflow.computeDataFlow();
  dataflow.dump();
}

void DfMainChecker::checkASTDecl(const TranslationUnitDecl* CTUD,
                                 AnalysisManager& Mgr, BugReporter& BR) const {
  TranslationUnitDecl* TUD = (TranslationUnitDecl*)CTUD;
  // fill data structures

  ASTContext& astContext = Mgr.getASTContext();
  MainWalker mainWalker(mainVars, mainFncs, astContext);
  mainWalker.TraverseDecl(TUD);
  mainWalker.finalize();

  mainVars.dump();
  mainFncs.dump();
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::DfMainChecker>(
      CHECKER_PLUGIN_NAME, "Data flow main checker", "");
}
