//===-- WriteChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "WriteChecker.h"

namespace clang::ento::nvm {

void WriteChecker::checkASTDecl(const TranslationUnitDecl* CTUD,
                                AnalysisManager& Mgr, BugReporter& BR) const {
  TranslationUnitDecl* TUD = (TranslationUnitDecl*)CTUD;
  // fill data structures
  TUDWalker tudWalker(varInfos, fncInfos);
  tudWalker.TraverseDecl(TUD);
  tudWalker.createUsedVars();

  fncInfos.dump();
  varInfos.dump();
}

void WriteChecker::checkBeginFunction(CheckerContext& C) const {
  bool isAnnotated = fncInfos.isAnnotatedFunction(C);
  bool isTopFnc = isTopFunction(C);

  // if not an annotated function, do not analyze
  if (!isAnnotated && isTopFnc) {
    ExplodedNode* ErrNode = C.generateErrorNode();
    if (!ErrNode)
      return;
  }
}

template <typename SMap>
void WriteChecker::checkMapStates(ProgramStateRef& State,
                                  CheckerContext& C) const {
  for (auto& [D, SS] : State->get<SMap>()) {
    if (!SS.isFinal()) {
      auto RI =
          ReportInfos::getRI(C, State, D, ErrNode, BReporter, nullptr, nullptr);
      RI.reportModelBug(SS.getExplanation());
    }
  }
}

void WriteChecker::checkEndFunction(CheckerContext& C) const {
  bool isAnnotated = fncInfos.isAnnotatedFunction(C);
  bool isTopFnc = isTopFunction(C);
  if (isAnnotated && isTopFnc) {
    // ensured it is the top function and annotated
    ProgramStateRef State = C.getState();
    ErrNode = nullptr;

    checkMapStates<CheckMap>(State, C);
    checkMapStates<DclMap>(State, C);
    checkMapStates<SclMap>(State, C);
  }
}
/*
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
*/

void WriteChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                             CheckerContext& C) const {
  // S->dump();
  ProgramStateRef State = C.getState();
  ErrNode = nullptr;
  bool stateChanged = false;

  if (const ValueDecl* VD = getValueDecl(Loc); VD) {
    if (varInfos.isUsedVar(VD)) {
      DBG("write " << VD->getNameAsString())
      auto& infoList = varInfos.getInfoList(VD);
      for (auto& BI : infoList) {
        auto RI = ReportInfos::getRI(C, State, (const char*)VD, ErrNode,
                                     BReporter, &Loc, S);
        BI->write(RI);
        stateChanged |= RI.stateChanged;
      }
    }
  }

  addStateTransition(State, C, stateChanged);
}

void WriteChecker::checkPreCall(const CallEvent& Call,
                                CheckerContext& C) const {

  if (fncInfos.isFlushFunction(Call)) {
    handleFlush(Call, C);
  } else if (fncInfos.isPFenceFunction(Call)) {
    handleFence<true>(Call, C);
  } else if (fncInfos.isVFenceFunction(Call)) {
    handleFence<false>(Call, C);
  }
}

void WriteChecker::handleFlush(const CallEvent& Call, CheckerContext& C) const {
  if (Call.getNumArgs() > 2) {
    llvm::report_fatal_error("check flush function");
    return;
  }

  ProgramStateRef State = C.getState();
  ErrNode = nullptr;
  bool stateChanged = false;

  SVal Loc = Call.getArgSVal(0);
  if (const ValueDecl* VD = getValueDecl(Loc); VD) {
    if (varInfos.isUsedVar(VD)) {
      DBG("flush " << VD->getNameAsString())
      auto& infoList = varInfos.getInfoList(VD);
      for (auto& BI : infoList) {
        auto RI = ReportInfos::getRI(C, State, (const char*)VD, ErrNode,
                                     BReporter, &Loc, nullptr);
        BI->flush(RI);
        stateChanged |= RI.stateChanged;
      }
    }
  }

  addStateTransition(State, C, stateChanged);
}

template <typename SMap, bool pfence>
void WriteChecker::checkFenceStates(ProgramStateRef& State, CheckerContext& C,
                                     bool& stateChanged) const {
  for (auto& [D, _] : State->get<SMap>()) {
    DBG("pfence " << (void*)D)
    // todo optimize for repeats
    auto& infoList = varInfos.getInfoList(D);
    for (auto& BI : infoList) {
      auto RI =
          ReportInfos::getRI(C, State, D, ErrNode, BReporter, nullptr, nullptr);
      
      if constexpr (pfence==true)
        BI->pfence(RI);
      else
        BI->vfence(RI);


      stateChanged |= RI.stateChanged;
    }
  }
}

template <bool pfence>
void WriteChecker::handleFence(const CallEvent& Call,
                                CheckerContext& C) const {

  ProgramStateRef State = C.getState();
  ErrNode = nullptr;
  bool stateChanged = false;

  checkFenceStates<CheckMap, pfence>(State, C, stateChanged);
  checkFenceStates<DclMap, pfence>(State, C, stateChanged);
  checkFenceStates<SclMap, pfence>(State, C, stateChanged);

  addStateTransition(State, C, stateChanged);
}

/*

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

void WriteChecker::handleVFence(const CallEvent& Call,
                                   CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  bool stateModified = sclVFenceTrans(State);

  if (stateModified) {
    C.addTransition(State);
  }
}
*/

void WriteChecker::addStateTransition(ProgramStateRef& State, CheckerContext& C,
                                      bool stateChanged) const {
  if (stateChanged) {
    C.addTransition(State);
  }
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::WriteChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
