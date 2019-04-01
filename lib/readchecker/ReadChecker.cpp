//===-- OrderingChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "ReadChecker.h"

namespace clang::ento::nvm {

/**
 * Skip analysis of unimportant functions
 */
void ReadChecker::checkBeginFunction(CheckerContext& C) const {
  bool isAnnotated = nvmFncInfo.isPersistentFunction(C);
  bool isTopFnc = isTopFunction(C);

  // if not an annotated function, do not analyze
  if (!isAnnotated && isTopFnc) {
    ExplodedNode* ErrNode = C.generateErrorNode();
    if (!ErrNode)
      return;
  }
}

void ReadChecker::checkDeadSymbols(SymbolReaper& SymReaper,
                                       CheckerContext& C) const {
  // todo implement
  // llvm::outs() << "consider implementing checkDeadSymbols\n";
}

ProgramStateRef ReadChecker::checkPointerEscape(
    ProgramStateRef State, const InvalidatedSymbols& Escaped,
    const CallEvent* Call, PointerEscapeKind Kind) const {
  // todo implement
  // llvm::outs() << "consider implementing checkPointerEscape\n";
  return State;
}

void ReadChecker::checkLocation(SVal Loc, bool IsLoad, const Stmt* S,
                                    CheckerContext& C) const {
  // only interested in reads
  if (!IsLoad) {
    return;
  }

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
}

void ReadChecker::handleReadData(SVal Loc, CheckerContext& C,
                                     const DeclaratorDecl* DD,
                                     DataInfo* DI) const {

  bool checked = recReadDataTrans(C, DD, DI);

  if (!checked) {
    ExplodedNode* ErrNode = C.generateNonFatalErrorNode();
    if (!ErrNode) {
      return;
    }
    BReporter.reportReadBug(Loc, C, DD, ErrNode, C.getBugReporter());
  }
}

void ReadChecker::handleReadCheck(CheckerContext& C,
                                      const DeclaratorDecl* DD,
                                      CheckInfo* CI) const {
  ProgramStateRef State = C.getState();
  bool stateModified = recReadCheckTrans(State, DD, CI);

  if (stateModified) {
    C.addTransition(State);
  }
}

void ReadChecker::handleReadMask(SVal Loc, const Stmt* S, CheckerContext& C,
                                     const DeclaratorDecl* DD,
                                     CheckDataInfo* CDI) const {

  // go up one level to get the full expression
  const Stmt* PS = getParentStmt(S, C);
  
  if (usesMask(PS, CDI->getMask(), true)) {
    // access validity part
    auto* CI = static_cast<CheckInfo*>(CDI);
    handleReadCheck(C, DD, CI);
  } else {
    // access data part - guaranteeed to be scl
    // todo fix leak
    DataInfo* DI = CDI->getDI();
    handleReadData(Loc, C, DD, DI);
  }
}

void ReadChecker::checkASTDecl(const FunctionDecl* FD, AnalysisManager& Mgr,
                                   BugReporter& BR) const {
  nvmFncInfo.insertIfKnown(FD);
}

void ReadChecker::checkASTDecl(const DeclaratorDecl* D,
                                   AnalysisManager& Mgr,
                                   BugReporter& BR) const {
  nvmTypeInfo.analyzeMemLabel(D);
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::ReadChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
