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
  DBG("checkBeginFunction")
  bool isAnnotated = fncInfos.isPersistentFunction(C);
  bool isTopFnc = isTopFunction(C);

  // if not an annotated function, do not analyze
  if (!isAnnotated && isTopFnc) {
    handleEnd(C);
  }
}

void WriteChecker::handleEnd(CheckerContext& C) const {
  ExplodedNode* ErrNode = C.generateErrorNode();
  if (!ErrNode)
    return;
}

template <typename SMap>
void WriteChecker::checkMapStates(ProgramStateRef& State,
                                  CheckerContext& C) const {
  DBG("checkMapStates")
  for (auto& [D, SS] : State->get<SMap>()) {
    if (!SS.isFinal()) {
      auto RI = ReportInfos::getRI(C, State, D, BReporter, nullptr, nullptr);
      RI.reportModelBug(SS.getExplanation());
    }
  }
}

void WriteChecker::checkEndFunction(CheckerContext& C) const {
  DBG("checkEndFunction")
  bool isAnnotated = fncInfos.isPersistentFunction(C);
  bool isTopFnc = isTopFunction(C);
  if (isAnnotated && isTopFnc) {
    // ensured it is the top function and annotated
    ProgramStateRef State = C.getState();

    checkMapStates<CheckMap>(State, C);
    checkMapStates<DclMap>(State, C);
    checkMapStates<SclMap>(State, C);
  }
}

template <typename SMap>
void WriteChecker::printStates(ProgramStateRef& State,
                               CheckerContext& C) const {
  DBG("printStates")
  for (auto& [D, SS] : State->get<SMap>()) {
    DBG((void*)D << " " << SS.getStateName())
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
  DBG("checkBind")
  // S->dump();
  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  if (const ValueDecl* VD = getValueDecl(Loc); VD) {
    if (varInfos.isUsedVar(VD)) {
      DBG("write " << VD->getNameAsString())
      auto& infoList = varInfos.getInfoList(VD);
      for (auto& BI : infoList) {
        auto RI =
            ReportInfos::getRI(C, State, (const char*)VD, BReporter, &Loc, S);
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
  } else if (fncInfos.isEndFunction(Call)) {
    handleEnd(C);
  }
}

void WriteChecker::handleFlush(const CallEvent& Call, CheckerContext& C) const {
  DBG("handleFlush")

  if (Call.getNumArgs() > 2) {
    llvm::report_fatal_error("check flush function");
    return;
  }

  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  SVal Loc = Call.getArgSVal(0);
  if (const ValueDecl* VD = getValueDecl(Loc); VD) {
    if (varInfos.isUsedVar(VD)) {
      DBG("flush " << VD->getNameAsString())
      auto& infoList = varInfos.getInfoList(VD);
      for (auto& BI : infoList) {
        auto RI = ReportInfos::getRI(C, State, (const char*)VD, BReporter, &Loc,
                                     nullptr);
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
    DBG("fence " << (void*)D)
    // todo optimize for repeats
    auto& infoList = varInfos.getInfoList(D);
    for (auto& BI : infoList) {
      auto RI = ReportInfos::getRI(C, State, D, BReporter, nullptr, nullptr);

      if constexpr (pfence == true)
        BI->pfence(RI);
      else
        BI->vfence(RI);

      stateChanged |= RI.stateChanged;
    }
  }
}

template <bool pfence>
void WriteChecker::handleFence(const CallEvent& Call, CheckerContext& C) const {
  DBG("handleFence")
  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  checkFenceStates<CheckMap, pfence>(State, C, stateChanged);
  checkFenceStates<DclMap, pfence>(State, C, stateChanged);
  checkFenceStates<SclMap, pfence>(State, C, stateChanged);

  addStateTransition(State, C, stateChanged);
}

void WriteChecker::checkBranchCondition(const Stmt* S,
                                        CheckerContext& C) const {
  //S->dump();
  if(usesName(S, "useNvm")){
    handleEnd(C);
  }
}

void WriteChecker::addStateTransition(ProgramStateRef& State, CheckerContext& C,
                                      bool stateChanged) const {
  if (stateChanged) {
    DBG("state transition")
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
