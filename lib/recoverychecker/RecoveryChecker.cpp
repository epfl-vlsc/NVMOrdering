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

/**
 * Check if model is correctly done
 */
void RecoveryChecker::checkEndFunction(CheckerContext& C) const {
  bool isAnnotated = nvmFncInfo.isAnnotatedFunction(C);
  bool isTopFnc = isTopFunction(C);
  if (isAnnotated && isTopFnc) {
    // ensured it is the top function and annotated
    ProgramStateRef State = C.getState();
    bool errNodeGenerated = false;
    ExplodedNode* ErrNode = nullptr;

    // iterate over rec
    for (auto& [checkDD, recState] : State->get<RecMap>()) {
      llvm::outs() << checkDD << " " << checkDD->getName() << " "
                   << recState.getStateName() << "\n";
      if (!recState.isReadData()) {
        if (!errNodeGenerated) {
          ErrNode = C.generateNonFatalErrorNode();
          errNodeGenerated = true;
        }

        // might return nullptr based on optimizations
        if (!ErrNode) {
          return;
        }

        BugInfo BI{checkDD, recState.getDataInfo(), recState.getStateName(),
                   recState.getStateKind()};
        BReporter.reportModelBug(C, BI, ErrNode, C.getBugReporter());
      }
    }
  }
}

void RecoveryChecker::checkDeadSymbols(SymbolReaper& SymReaper,
                                       CheckerContext& C) const {
  // todo implement
  // llvm::outs() << "consider implementing checkDeadSymbols\n";
}

ProgramStateRef OrderingChecker::checkPointerEscape(
    ProgramStateRef State, const InvalidatedSymbols& Escaped,
    const CallEvent* Call, PointerEscapeKind Kind) const {
  // todo implement
  // llvm::outs() << "consider implementing checkPointerEscape\n";
  return State;
}

void RecoveryChecker::checkLocation(SVal Loc, bool IsLoad, const Stmt* S,
                                    CheckerContext&) const {
  // only interested in reads
  if (!isLoad) {
    return;
  }
  // S->dump();
  const MemRegion* Region = Loc.getAsRegion();
  if (const FieldRegion* FieldReg = Region->getAs<FieldRegion>()) {
    const Decl* BD = FieldReg->getDecl();
    const DeclaratorDecl* D = getDeclaratorDecl(BD);

    if (nvmTypeInfo.inLabels(D)) {
      LabeledInfo* LI = nvmTypeInfo.getDeclaratorInfo(D);
      if (LI->isCheck()) {
        auto* I = static_cast<CheckInfo*>(LI);
        if (I->hasMask()) {
          // bit field special case
          // can be data or check, if it is data, treat is as data
          // if it is check, treat it as check
          auto* CDI = static_cast<CheckDataInfo*>(I);
          // handle read mask
        } else {
          // handle read check
        }
      } else {
        auto* I = static_cast<DataInfo*>(LI);
        // handle read data
      }
    }
  }
}

void RecoveryChecker::checkBranchCondition(const Stmt* Condition, CheckerContext& Ctx) const {
  llvm::outs() << "branch condition\n";
}

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
  registry.addChecker<clang::ento::nvm::OrderingChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
