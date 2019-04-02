//===-- OrderingChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "ReadChecker.h"

namespace clang::ento::nvm {

void ReadChecker::checkASTDecl(const TranslationUnitDecl* CTUD,
                               AnalysisManager& Mgr, BugReporter& BR) const {
  TranslationUnitDecl* TUD = (TranslationUnitDecl*)CTUD;
  // fill data structures
  TUDWalker tudWalker(varInfos, fncInfos);
  tudWalker.TraverseDecl(TUD);
  tudWalker.createUsedVars();

  fncInfos.dump();
  varInfos.dump();
}

void ReadChecker::checkBeginFunction(CheckerContext& C) const {
  bool isAnnotated = fncInfos.isRecoveryFunction(C);
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

  /*
  const MemRegion* Region = Loc.getAsRegion();
  if (const FieldRegion* FieldReg = Region->getAs<FieldRegion>()) {
    const Decl* BD = FieldReg->getDecl();
    const DeclaratorDecl* DD = getDeclaratorDecl(BD);

    if (nvmTypeInfo.inLabels(DD)) {
      LabeledInfo* LI = nvmTypeInfo.getDeclaratorInfo(DD);
      if (LI->isCheck()) {
        auto* CI = static_cast<CheckInfo*>(LI);
        if (CI->hasMask()) {
          // bit field special case
          // can be data or check, if it is data, treat is as data
          // if it is check, treat it as check

          auto* CDI = static_cast<CheckDataInfo*>(CI);
          handleReadMask(Loc, S, C, DD, CDI);
        } else {
          handleReadCheck(C, DD, CI);
        }
      } else {
        auto* DI = static_cast<DataInfo*>(LI);
        handleReadData(Loc, C, DD, DI);
      }
    }
  }
  */
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::ReadChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
