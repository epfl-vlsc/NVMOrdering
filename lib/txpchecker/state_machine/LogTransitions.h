#pragma once
#include "Common.h"
#include "IpTransitions.h"
#include "StateInfo.h"
#include "States.h"

namespace clang::ento::nvm::LogSpace {

bool seenFirst(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  const Stmt* S = SI.S;
  SourceRange SR = S->getSourceRange();

  return !SlSpace::isSRinStore(State, SR);
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
    State = State->add<LogVarMap>(VI);
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