#pragma once
#include "Common.h"
#include "DbgState.h"
#include "LogState.h"
#include "StateInfo.h"

namespace clang::ento::nvm::LogSpace {

void writeData(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  const LogState* FieldLogState = State->get<LogMap>(SI.ND);

  if (!FieldLogState) {
    DBG("noexist")
    SI.reportNotLogBeforeWriteBug();
  } else if (FieldLogState->isInit()) {
    DBG("init")
    // todo
  } else if (FieldLogState->isLogged()) {
    DBG("logged")
    // correct do nothing
  } else if (FieldLogState->isWritten()) {
    DBG("written")
    // todo
  } else {
    llvm::report_fatal_error("impossible write");
  }
}

void logData(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  const LogState* FieldLogState = State->get<LogMap>(SI.ND);

  if (!FieldLogState) {
    DBG("noexist")
    // correct, do nothing
    State = State->set<LogMap>(SI.ND, LogState::getLogged());
    SI.addTransition();
  } else if (FieldLogState->isInit()) {
    DBG("init")
    // to do
  } else if (FieldLogState->isLogged()) {
    DBG("logged")
    SI.reportDoubleLogBug();
  } else if (FieldLogState->isWritten()) {
    DBG("written")
  } else {
    llvm::report_fatal_error("impossible write");
  }
}

} // namespace clang::ento::nvm::LogSpace