//===-- WriteChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "WriteChecker.h"

namespace clang::ento::nvm {

void WriteChecker::checkASTDecl(const TranslationUnitDecl* CTUD,
                                AnalysisManager& Mgr, BugReporter& BR) const {
  TranslationUnitDecl* TUD = (TranslationUnitDecl*)CTUD;
  // fill data structures
  WriteWalker tudWalker(orderVars, orderFncs);
  tudWalker.TraverseDecl(TUD);
  tudWalker.createUsedVars();

  orderVars.dump();
  orderFncs.dump();
}

void WriteChecker::checkBeginFunction(CheckerContext& C) const {
  DBG("checkBeginFunction")
  bool isAnnotated = orderFncs.isPersistentFunction(C);
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
      auto SI = StateInfo(C, State, BReporter, nullptr, nullptr, D);
      SI.reportModelBug(SS.getExplanation());
    }
  }
}

void WriteChecker::checkEndFunction(CheckerContext& C) const {
  DBG("checkEndFunction")
  bool isAnnotated = orderFncs.isPersistentFunction(C);
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

void WriteChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                             CheckerContext& C) const {
  DBG("checkBind")
  // S->dump();
  ProgramStateRef State = C.getState();
  bool stateChanged = false;

  if (const ValueDecl* VD = getValueDecl(Loc); VD) {
    if (orderVars.isUsedVar(VD)) {
      DBG("write " << VD->getNameAsString())
      auto& infoList = orderVars.getInfoList(VD);
      for (auto& BI : infoList) {
        auto SI = StateInfo(C, State, BReporter, &Loc, S, (const char*)VD);
        BI->write(SI);
        stateChanged |= SI.stateChanged;
      }
    }
  }

  addStateTransition(State, C, stateChanged);
}

void WriteChecker::checkPreCall(const CallEvent& Call,
                                CheckerContext& C) const {
  if (orderFncs.isFlushFunction(Call)) {
    handleFlush(Call, C);
  } else if (orderFncs.isPFenceFunction(Call)) {
    handleFence<true>(Call, C);
  } else if (orderFncs.isVFenceFunction(Call)) {
    handleFence<false>(Call, C);
  } else if (orderFncs.isEndFunction(Call)) {
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
    if (orderVars.isUsedVar(VD)) {
      DBG("flush " << VD->getNameAsString())
      auto& infoList = orderVars.getInfoList(VD);
      for (auto& BI : infoList) {
        auto SI =
            StateInfo(C, State, BReporter, &Loc, nullptr, (const char*)VD);
        BI->flush(SI);
        stateChanged |= SI.stateChanged;
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
    auto& infoList = orderVars.getInfoList(D);
    for (auto& BI : infoList) {
      auto SI =
          StateInfo(C, State, BReporter, nullptr, nullptr, D);

      if constexpr (pfence == true)
        BI->pfence(SI);
      else
        BI->vfence(SI);

      stateChanged |= SI.stateChanged;
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
  // S->dump();
  if (usesName(S, "useNvm")) {
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
