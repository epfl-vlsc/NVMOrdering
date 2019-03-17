//===-- OrderingChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "RecoveryChecker.h"

namespace clang::ento::nvm {

/**
 * Skip analysis of unimportant functions
 */
void RecoveryChecker::checkBeginFunction(CheckerContext& C) const {
  bool isAnnotated = nvmFncInfo.isAnnotatedFunction(C);
  bool isTopFnc = isTopFunction(C);

  // if not an annotated function, do not analyze
  if (!isAnnotated && isTopFnc) {
    ExplodedNode* ErrNode = C.generateErrorNode();
    if (!ErrNode)
      return;
  }
}

void RecoveryChecker::checkDeadSymbols(SymbolReaper& SymReaper,
                                       CheckerContext& C) const {
  // todo implement
  // llvm::outs() << "consider implementing checkDeadSymbols\n";
}

ProgramStateRef RecoveryChecker::checkPointerEscape(
    ProgramStateRef State, const InvalidatedSymbols& Escaped,
    const CallEvent* Call, PointerEscapeKind Kind) const {
  // todo implement
  // llvm::outs() << "consider implementing checkPointerEscape\n";
  return State;
}

void RecoveryChecker::checkLocation(SVal Loc, bool IsLoad, const Stmt* S,
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
        auto* I = static_cast<CheckInfo*>(LI);
        if (I->hasMask()) {
          // bit field special case
          // can be data or check, if it is data, treat is as data
          // if it is check, treat it as check
          auto* CDI = static_cast<CheckDataInfo*>(I);
          // handle read mask
        } else {
          handleReadCheck(C, DD, I);
        }
      } else {
        auto* I = static_cast<DataInfo*>(LI);
        handleReadData(Loc, C, DD, I);
      }
    }
  }
}

void RecoveryChecker::handleReadData(SVal Loc, CheckerContext& C,
                                     const DeclaratorDecl* DD,
                                     DataInfo* DI) const {
  ProgramStateRef State = C.getState();
  bool checked = false;

  for (auto& [checkDD, recState] : State->get<RecMap>()) {
    StringRef checkName = checkDD->getName();
    if (DI->isSameCheckName(checkName)) {
      // pair
      checked = true;
      if (recState.isReadCheck()) {
        llvm::outs() << "lol\n";
        recReadDataTrans(State, checkDD, recState.getCheckInfo());
        C.addTransition(State);
      } else {
        // check already read
      }

    } else {
      // not pair
    }
  }

  if (!checked) {
    ExplodedNode* ErrNode = C.generateNonFatalErrorNode();
    if (!ErrNode) {
      return;
    }
    BReporter.reportReadBug(Loc, C, DD, ErrNode, C.getBugReporter());
  }
}

void RecoveryChecker::handleReadCheck(CheckerContext& C,
                                      const DeclaratorDecl* DD,
                                      CheckInfo* CI) const {
  ProgramStateRef State = C.getState();
  bool stateModified = recReadCheckTrans(State, DD, CI);

  if (stateModified) {
    C.addTransition(State);
  }
}

void RecoveryChecker::handleReadMask(CheckerContext& C,
                                     const DeclaratorDecl* DD,
                                     CheckInfo* CI) const {}

void RecoveryChecker::checkASTDecl(const FunctionDecl* FD, AnalysisManager& Mgr,
                                   BugReporter& BR) const {
  nvmFncInfo.insertIfKnown(FD);
}

void RecoveryChecker::checkASTDecl(const DeclaratorDecl* D,
                                   AnalysisManager& Mgr,
                                   BugReporter& BR) const {
  nvmTypeInfo.analyzeMemLabel(D);
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::RecoveryChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
