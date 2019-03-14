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

void OrderingChecker::checkBind(SVal Loc, SVal Val, const Stmt *S,
                                CheckerContext &C) const
{

  Loc.dump();
  llvm::outs() << "lol bind\n";
  

  const TypedValueRegion *TR =
      dyn_cast_or_null<TypedValueRegion>(Loc.getAsRegion());
  if (!TR)
  {
    return;
  }

  QualType valTy = TR->getValueType();
  valTy.dump();
  llvm::outs() << "\n";

  /*
  llvm::outs() << "bind\n";
  Loc.dump();

  const TypedValueRegion *TR =
      dyn_cast_or_null<TypedValueRegion>(Loc.getAsRegion());

  if (!TR)
    return;

  llvm::outs() << "\n";
  QualType valTy = TR->getValueType();
  valTy.dump();

  llvm::outs() << "\n";
  Val.dump();
  llvm::outs() << "\n";
  S->dump();
  llvm::outs() << "\n";
  */
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
