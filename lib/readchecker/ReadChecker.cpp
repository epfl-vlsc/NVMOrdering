//===-- OrderingChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "ReadChecker.h"

namespace clang::ento::nvm {

void ReadChecker::checkASTDecl(const TranslationUnitDecl* CTUD,
                               AnalysisManager& Mgr, BugReporter& BR) const {
  TranslationUnitDecl* TUD = (TranslationUnitDecl*)CTUD;
  // fill data structures
  const ASTContext& ASTC = Mgr.getASTContext();
  ReadWalker tudWalker(orderVars, orderFncs, ASTC);
  tudWalker.TraverseDecl(TUD);
  tudWalker.createUsedVars();

  orderFncs.dump();
  orderVars.dump();
}

void ReadChecker::checkBeginFunction(CheckerContext& C) const {
  bool isAnnotated = orderFncs.isRecoveryFunction(C);
  bool isTopFnc = isTopFunction(C);

  // if not an annotated function, do not analyze
  if (!isAnnotated && isTopFnc) {
    ExplodedNode* ErrNode = C.generateErrorNode();
    if (!ErrNode)
      return;
  }
}

void ReadChecker::checkLocation(SVal Loc, bool IsLoad, const Stmt* S,
                                CheckerContext& C) const {
  // only interested in reads
  if (!IsLoad) {
    return;
  }

  DBG("checkLocation")
  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  // get the statement where mask can exist
  const Stmt* PS = getParentStmt(S, C);
  // PS->dump();

  if (const ValueDecl* VD = getValueDecl(Loc); VD) {
    if (orderVars.isUsedVar(VD)) {
      DBG("read " << VD->getNameAsString())
      auto& infoList = orderVars.getInfoList(VD);
      for (auto& BI : infoList) {
        auto SI = StateInfo(C, State, BReporter, &Loc, PS, (const char*)VD);
        BI->read(SI);
        stateChanged |= SI.stateChanged;
      }
    }
  }

  addStateTransition(State, C, stateChanged);
}

void ReadChecker::addStateTransition(ProgramStateRef& State, CheckerContext& C,
                                     bool stateChanged) const {
  if (stateChanged) {
    DBG("state transition")
    C.addTransition(State);
  }
}

void ReadChecker::handleEnd(CheckerContext& C) const {
  ExplodedNode* ErrNode = C.generateErrorNode();
  if (!ErrNode)
    return;
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::ReadChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
