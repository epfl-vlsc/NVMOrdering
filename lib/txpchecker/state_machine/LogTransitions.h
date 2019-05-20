#pragma once
#include "Common.h"
#include "IpTransitions.h"
#include "SrTransitions.h"
#include "StateInfo.h"
#include "States.h"

namespace clang::ento::nvm::LogSpace {

bool seenFirst(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  const Stmt* S = SI.S;
  SourceRange SR = S->getSourceRange();

  /*
    CheckerContext& C = SI.C;
    const SourceManager &SM = C.getSourceManager();
    SR.dump(SM);
  */
  return !SrSpace::isSRInSlStore(State, SR);
}

bool removeLogFromMap(ProgramStateRef& State, const FunctionDecl* FD) {
  bool stateChanged = false;
  for (auto& VI : State->get<LogVarMap>()) {
    if (VI.isSameFnc(FD)) {
      State = State->remove<LogVarMap>(VI);
      stateChanged = true;
    }
  }
  return stateChanged;
}

void printLogMap(ProgramStateRef& State) {
  for (auto& VI : State->get<LogVarMap>()) {
    VI.dump("log");
  }
}

void logData(StateInfo& SI) {
  ProgramStateRef& State = SI.State;

  auto& VI = SI.getVI();
  auto vList = IpSpace::getVarBaseList(State, VI);
  dumpVector(vList);

  bool reportBug = false;

  // todo not covering node->log, root->log case
  for (auto& vElement : vList) {
    if (State->contains<LogVarMap>(vElement)) {
      DBG("obj logged")
      if (seenFirst(SI)) {
        SI.reportDoubleLogBug();
        reportBug = true;
      }
    }
  }

  if (!reportBug) {
    DBG("obj not logged")
    auto VarVI = IpSpace::getTransitiveVar(State, VI);
    State = State->add<LogVarMap>(VarVI);
    SI.stateChanged = true;
  }
}

void writeData(StateInfo& SI) {
  ProgramStateRef& State = SI.State;

  auto& VI = SI.getVI();
  auto vList = IpSpace::getVarBaseList(State, VI);
  dumpVector(vList);

  bool baseNotLog = true;
  for (auto& vElement : vList) {
    if (State->contains<LogVarMap>(vElement)) {
      baseNotLog = false;
    }
  }

  if (baseNotLog) {
    DBG("obj logged")
    SI.reportNotLogBeforeWriteBug();
  }
}

} // namespace clang::ento::nvm::LogSpace