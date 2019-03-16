//===-- OrderingChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "OrderingChecker.h"

namespace clang::ento::nvm {

/**
 * Skip analysis of unimportant functions
 */
void OrderingChecker::checkBeginFunction(CheckerContext& C) const {
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
void OrderingChecker::checkEndFunction(CheckerContext& C) const {
  bool isAnnotated = nvmFncInfo.isAnnotatedFunction(C);
  bool isTopFnc = isTopFunction(C);
  if (isAnnotated && isTopFnc) {
    // ensured it is the top function and annotated

    ProgramStateRef State = C.getState();
    // iterate over dcl
    for (auto& [dataDeclDecl, dclState] : State->get<DclMap>()) {

      llvm::outs() << dataDeclDecl << " dcl " << dataDeclDecl->getName() << " "
                   << dclState.getStateName() << "\n";
      if (!dclState.isPFenceCheck()) {
        ExplodedNode* ErrNode = C.generateNonFatalErrorNode();
        if (!ErrNode) {
          return;
        }
        BReporter.reportModelBug(C, ErrNode, C.getBugReporter());
      }
    }

    // iterate over scl
    for (auto& [dataDeclDecl, sclState] : State->get<SclMap>()) {
      llvm::outs() << dataDeclDecl << " scl " << dataDeclDecl->getName() << " "
                   << sclState.getStateName() << "\n";
      if (!sclState.isWriteCheck()) {
        ExplodedNode* ErrNode = C.generateNonFatalErrorNode();
        if (!ErrNode) {
          return;
        }
        BReporter.reportModelBug(C, ErrNode, C.getBugReporter());
      }
    }
  }
}

void OrderingChecker::checkDeadSymbols(SymbolReaper& SymReaper,
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

void OrderingChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                                CheckerContext& C) const {
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
          handleWriteMask(S, C, D, CDI);
        } else {
          handleWriteCheck(C, D, I);
        }
      } else {
        auto* I = static_cast<DataInfo*>(LI);
        handleWriteData(C, D, I);
      }
    }
  }
}

void OrderingChecker::handleWriteData(CheckerContext& C,
                                      const DeclaratorDecl* D,
                                      DataInfo* DI) const {
  ProgramStateRef State = C.getState();
  bool stateModified = false;

  // check if not completed or not initial throw error
  if (DI->isDcl()) {
    // different cache line
    stateModified |= dclWriteDataTrans(State, D, DI);
  } else {
    // same cache line
    stateModified |= sclWriteDataTrans(State, D, DI);
  }

  if (stateModified) {
    C.addTransition(State);
  }
}

void OrderingChecker::handleWriteCheck(CheckerContext& C,
                                       const DeclaratorDecl* D,
                                       CheckInfo* CI) const {
  ProgramStateRef State = C.getState();
  StringRef checkName = D->getName();

  bool stateModified = dclWriteCheckTrans(State, checkName);

  stateModified |= sclWriteCheckTrans(State, checkName);

  if (stateModified) {
    C.addTransition(State);
  } else {
    ExplodedNode* ErrNode = C.generateNonFatalErrorNode();
    if (!ErrNode) {
      return;
    }
    BReporter.reportWriteBug(C, D, ErrNode, C.getBugReporter());
  }
}

void OrderingChecker::handleWriteMask(const Stmt* S, CheckerContext& C,
                                      const DeclaratorDecl* D,
                                      CheckDataInfo* CDI) const {
  if (usesMask(S, CDI->getMask())) {
    // access validity part
    auto* I = static_cast<CheckInfo*>(CDI);
    handleWriteCheck(C, D, I);
  } else {
    // access data part - guaranteeed to be scl
    ProgramStateRef State = C.getState();

    bool stateModified = sclWriteMaskTrans(State, D, CDI);

    if (stateModified) {
      C.addTransition(State);
    }
  }
}

void OrderingChecker::checkPreCall(const CallEvent& Call,
                                   CheckerContext& C) const {
  if (nvmFncInfo.isFlushFunction(Call)) {
    handleFlush(Call, C);
  } else if (nvmFncInfo.isPFenceFunction(Call)) {
    handlePFence(Call, C);
  } else if (nvmFncInfo.isVFenceFunction(Call)) {
    handleVFence(Call, C);
  }
}

void OrderingChecker::handleFlush(const CallEvent& Call,
                                  CheckerContext& C) const {
  if (Call.getNumArgs() > 2) {
    llvm::report_fatal_error("check flush function");
    return;
  }

  SVal Loc = Call.getArgSVal(0);
  const MemRegion* Region = Loc.getAsRegion();
  if (const FieldRegion* FieldReg = Region->getAs<FieldRegion>()) {
    const Decl* BD = FieldReg->getDecl();
    const DeclaratorDecl* D = getDeclaratorDecl(BD);
    if (nvmTypeInfo.inLabels(D)) {
      LabeledInfo* LI = nvmTypeInfo.getDeclaratorInfo(D);
      if (LI->isCheck()) {
        auto* I = static_cast<CheckInfo*>(LI);
        handleFlushCheck(C, D, I);
      } else {
        auto* I = static_cast<DataInfo*>(LI);
        handleFlushData(C, D, I);
      }
    }
  }
}

void OrderingChecker::handleFlushData(CheckerContext& C,
                                      const DeclaratorDecl* D,
                                      DataInfo* DI) const {
  ProgramStateRef State = C.getState();
  bool stateModified = false;
  if (DI->isDcl()) {
    stateModified |= dclFlushDataTrans(State, D, DI);
  }
  if (stateModified) {
    C.addTransition(State);
  }
}

void OrderingChecker::handleFlushCheck(CheckerContext& C,
                                       const DeclaratorDecl* D,
                                       CheckInfo* CI) const {
  ProgramStateRef State = C.getState();
  StringRef checkName = D->getName();
  bool stateModified = dclFlushCheckTrans(State, checkName);

  if (stateModified) {
    C.addTransition(State);
  }
}

void OrderingChecker::handlePFence(const CallEvent& Call,
                                   CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  bool stateModified = dclPFenceTrans(State);
  stateModified |= sclPFenceTrans(State);

  if (stateModified) {
    C.addTransition(State);
  }
}

void OrderingChecker::handleVFence(const CallEvent& Call,
                                   CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  bool stateModified = sclVFenceTrans(State);

  if (stateModified) {
    C.addTransition(State);
  }
}

void OrderingChecker::checkASTDecl(const FunctionDecl* D, AnalysisManager& Mgr,
                                   BugReporter& BR) const {
  nvmFncInfo.checkFunction(D);
}

void OrderingChecker::checkASTDecl(const DeclaratorDecl* D,
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
