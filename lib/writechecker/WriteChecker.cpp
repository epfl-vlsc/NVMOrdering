//===-- WriteChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "WriteChecker.h"

namespace clang::ento::nvm {


void WriteChecker::checkASTDecl(const TranslationUnitDecl* TUD,
                              AnalysisManager& Mgr, BugReporter& BR) const {
  varInfos.collectUsedVars(TUD);
  //varInfos.dump();
}

/*
void WriteChecker::checkBeginFunction(CheckerContext& C) const {
  bool isAnnotated = nvmFncInfo.isAnnotatedFunction(C);
  bool isTopFnc = isTopFunction(C);

  // if not an annotated function, do not analyze
  if (!isAnnotated && isTopFnc) {
    ExplodedNode* ErrNode = C.generateErrorNode();
    if (!ErrNode)
      return;
  }
}

void WriteChecker::checkEndFunction(CheckerContext& C) const {
  bool isAnnotated = nvmFncInfo.isAnnotatedFunction(C);
  bool isTopFnc = isTopFunction(C);
  if (isAnnotated && isTopFnc) {
    // ensured it is the top function and annotated
    ProgramStateRef State = C.getState();
    bool errNodeGenerated = false;
    ExplodedNode* ErrNode = nullptr;

    llvm::outs() << "state values\n";
    // iterate over dcl
    for (auto& [dataDD, dclState] : State->get<DclMap>()) {
      llvm::outs() << getPairStr(dataDD, dclState.getDataInfo()) << " "
                   << dclState.getStateName() << "\n";
      if (!dclState.isPFenceCheck()) {
        if (!errNodeGenerated) {
          ErrNode = C.generateNonFatalErrorNode();
          errNodeGenerated = true;
        }

        // might return nullptr based on optimizations
        if (!ErrNode) {
          return;
        }

        BugInfo BI{dataDD, dclState.getDataInfo(), dclState.getStateName(),
                   dclState.getStateKind(), true};
        BReporter.reportModelBug(C, BI, ErrNode, C.getBugReporter());
      }
    }

    // iterate over scl
    for (auto& [dataDD, sclState] : State->get<SclMap>()) {
      llvm::outs() << getPairStr(dataDD, sclState.getDataInfo()) << " "
                   << sclState.getStateName() << "\n";
      if (!sclState.isWriteCheck()) {
        if (!errNodeGenerated) {
          ErrNode = C.generateNonFatalErrorNode();
          errNodeGenerated = true;
        }

        // might return nullptr based on optimizations
        if (!ErrNode) {
          return;
        }
        BugInfo BI{dataDD, sclState.getDataInfo(), sclState.getStateName(),
                   sclState.getStateKind(), false};
        BReporter.reportModelBug(C, BI, ErrNode, C.getBugReporter());
      }
    }
  }
}

void WriteChecker::checkDeadSymbols(SymbolReaper& SymReaper,
                                       CheckerContext& C) const {
  // todo implement
  // llvm::outs() << "consider implementing checkDeadSymbols\n";
}

ProgramStateRef WriteChecker::checkPointerEscape(
    ProgramStateRef State, const InvalidatedSymbols& Escaped,
    const CallEvent* Call, PointerEscapeKind Kind) const {
  // todo implement
  // llvm::outs() << "consider implementing checkPointerEscape\n";
  return State;
}

void WriteChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
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
          handleWriteMask(Loc, S, C, D, CDI);
        } else {
          handleWriteCheck(Loc, C, D, I);
        }
      } else {
        auto* I = static_cast<DataInfo*>(LI);
        handleWriteData(C, D, I);
      }
    }
  }
}

void WriteChecker::handleWriteData(CheckerContext& C,
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
  } else {
    // todo throw bug
  }
}

void WriteChecker::handleWriteCheck(SVal Loc, CheckerContext& C,
                                       const DeclaratorDecl* D,
                                       CheckInfo* CI) const {
  ProgramStateRef State = C.getState();
  StringRef checkName = D->getName();
  bool seen = false;

  bool stateModified = dclWriteCheckTrans(State, checkName, seen);

  stateModified |= sclWriteCheckTrans(State, checkName, seen);

  if (stateModified) {
    C.addTransition(State);
  } else if (!seen) {
    ExplodedNode* ErrNode = C.generateNonFatalErrorNode();
    if (!ErrNode) {
      return;
    }
    llvm::outs() << "report bug\n";
    BReporter.reportWriteBug(Loc, C, D, ErrNode, C.getBugReporter());
  } else {
    // todo throw bug
  }
}

void WriteChecker::handleWriteMask(SVal Loc, const Stmt* S,
                                      CheckerContext& C,
                                      const DeclaratorDecl* D,
                                      CheckDataInfo* CDI) const {
  if (usesMask(S, CDI->getMask(), false)) {
    // access validity part
    auto* I = static_cast<CheckInfo*>(CDI);
    handleWriteCheck(Loc, C, D, I);
  } else {
    // access data part - guaranteeed to be scl
    ProgramStateRef State = C.getState();

    bool stateModified = sclWriteMaskTrans(State, D, CDI);

    if (stateModified) {
      C.addTransition(State);
    } else {
      // todo throw bug
    }
  }
}

void WriteChecker::checkPreCall(const CallEvent& Call,
                                   CheckerContext& C) const {
  if (nvmFncInfo.isFlushFunction(Call)) {
    handleFlush(Call, C);
  } else if (nvmFncInfo.isPFenceFunction(Call)) {
    handlePFence(Call, C);
  } else if (nvmFncInfo.isVFenceFunction(Call)) {
    handleVFence(Call, C);
  }
}

void WriteChecker::handleFlush(const CallEvent& Call,
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

void WriteChecker::handleFlushData(CheckerContext& C,
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

void WriteChecker::handleFlushCheck(CheckerContext& C,
                                       const DeclaratorDecl* D,
                                       CheckInfo* CI) const {
  ProgramStateRef State = C.getState();
  StringRef checkName = D->getName();
  bool stateModified = dclFlushCheckTrans(State, checkName);

  if (stateModified) {
    C.addTransition(State);
  }
}

void WriteChecker::handlePFence(const CallEvent& Call,
                                   CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  bool stateModified = dclPFenceTrans(State);
  stateModified |= sclPFenceTrans(State);

  if (stateModified) {
    C.addTransition(State);
  }
}

void WriteChecker::handleVFence(const CallEvent& Call,
                                   CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  bool stateModified = sclVFenceTrans(State);

  if (stateModified) {
    C.addTransition(State);
  }
}

void WriteChecker::checkASTDecl(const FunctionDecl* FD, AnalysisManager& Mgr,
                                   BugReporter& BR) const {
  nvmFncInfo.insertIfKnown(FD);
}

void WriteChecker::checkASTDecl(const DeclaratorDecl* D,
                                   AnalysisManager& Mgr,
                                   BugReporter& BR) const {
  nvmTypeInfo.analyzeMemLabel(D);
}
*/
} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::WriteChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
