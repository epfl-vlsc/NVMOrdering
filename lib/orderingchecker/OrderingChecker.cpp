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

void OrderingChecker::checkDeadSymbols(SymbolReaper &SymReaper, CheckerContext &C) const
{
  //todo implement
  llvm::outs() << "consider implementing checkDeadSymbols\n";
}

ProgramStateRef OrderingChecker::checkPointerEscape(ProgramStateRef State,
                                                    const InvalidatedSymbols &Escaped,
                                                    const CallEvent *Call,
                                                    PointerEscapeKind Kind) const
{
  //todo implement
  llvm::outs() << "consider implementing checkPointerEscape\n";
  return State;
}

void OrderingChecker::checkBind(SVal Loc, SVal Val, const Stmt *S,
                                CheckerContext &C) const
{
  const MemRegion *Region = Loc.getAsRegion();
  if (const FieldRegion *FieldReg = Region->getAs<FieldRegion>())
  {
    FieldReg->dump();
    const Decl *BD = FieldReg->getDecl();
    const DeclaratorDecl *D = getDeclaratorDecl(BD);
    if (nvmTypeInfo.inLabels(D))
    {
      LabeledDecl *LD = nvmTypeInfo.getDeclaratorInfo(D);
      if (LD->isCheck())
      {
        auto *DD = static_cast<CheckDecl *>(LD);
        handleCheck(Loc, C, D, DD);
      }
      else
      {
        auto *DD = static_cast<DataDecl *>(LD);
        handleData(Loc, C, D, DD);
      }
    }
  }

  llvm::outs() << "\n";
}

void OrderingChecker::handleData(SVal &Loc, CheckerContext &C,
                                 const DeclaratorDecl *D, DataDecl *DD) const
{
  //check if not completed or not initial throw error

  //set state to wd
}

void OrderingChecker::handleCheck(SVal &Loc, CheckerContext &C,
                                  const DeclaratorDecl *D, CheckDecl *DD) const
{
  //iterate over all states, check if any valid bit exists
}

void OrderingChecker::checkPreCall(const CallEvent &Call,
                                   CheckerContext &C) const
{
  if (nvmFncInfo.isFlushFunction(Call))
  {
    handleFlush(Call, C);
  }
  else if (nvmFncInfo.isPFenceFunction(Call))
  {
    handlePFence(Call, C);
  }
  else if (nvmFncInfo.isVFenceFunction(Call))
  {
    handleVFence(Call, C);
  }
}

void OrderingChecker::handleFlush(const CallEvent &Call,
                                  CheckerContext &C) const
{
  llvm::outs() << "lol flush\n";
}

void OrderingChecker::handlePFence(const CallEvent &Call,
                                   CheckerContext &C) const
{
  llvm::outs() << "lol pfence\n";
}

void OrderingChecker::handleVFence(const CallEvent &Call,
                                   CheckerContext &C) const
{
  llvm::outs() << "lol vfence\n";
}

void OrderingChecker::checkASTDecl(const FunctionDecl *D, AnalysisManager &Mgr,
                                   BugReporter &BR) const
{
  nvmFncInfo.checkFunction(D);
}

void OrderingChecker::checkASTDecl(const DeclaratorDecl *D, AnalysisManager &Mgr,
                                   BugReporter &BR) const
{
  nvmTypeInfo.analyzeMemLabel(D);
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry &registry)
{
  registry.addChecker<clang::ento::nvm::OrderingChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
