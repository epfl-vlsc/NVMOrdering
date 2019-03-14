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
  
  const MemRegion *Region = Loc.getAsRegion();
  if (const FieldRegion* FieldReg = Region->getAs<FieldRegion>()){
    FieldReg->dump();
    const Decl* BD = FieldReg->getDecl();
    const DeclaratorDecl* D = getDeclaratorDecl(BD);
    if(nvmTypeInfo.inLabels(D)){
      LabeledDecl* LD = nvmTypeInfo.getDeclaratorInfo(D);
      if(LD->isCheck()){
        handleCheck(Loc, C, LD);
      }else{
        handleData(Loc, C, LD);
      }
    }
  }

  llvm::outs() << "\n";
}

void OrderingChecker::handleData(SVal& Loc, CheckerContext &C, LabeledDecl* LD) const{

}

void OrderingChecker::handleCheck(SVal& Loc, CheckerContext &C, LabeledDecl* LD) const{
  
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
