#pragma once
#include "Common.h"
#include "States.h"

namespace clang::ento::nvm {


/*
void codeTransition(ProgramStateRef& State, CheckerContext& C) {
  State = State->set<Logged>(true);
  C.addTransition(State);
}

void logTransition(const ValueDecl* VD, ProgramStateRef& State,
                   CheckerContext& C, const LogBugReporter& LBReporter) {
  bool codeActive = State->get<Logged>();
  if (codeActive) {
    return;
  }

  const WriteState* WS = State->get<LogMap>(VD);
  if (!WS) {
    // not exists
    State = State->set<LogMap>(VD, WriteState::getLogged());
    C.addTransition(State);
  } else if (WS->isLogged()) {
    // already logged
    LBReporter.reportAlreadyLogged(VD, C);
  } else if (WS->isWritten()) {
    // written
    State = State->set<LogMap>(VD, WriteState::getLogged());
    C.addTransition(State);
  } else {
    llvm::report_fatal_error("not possible transition");
  }
}

void writeTransition(const ValueDecl* VD, ProgramStateRef& State,
                     CheckerContext& C, const LogBugReporter& LBReporter) {
  bool codeActive = State->get<Logged>();
  if (codeActive) {
    return;
  }

  const WriteState* WS = State->get<LogMap>(VD);
  if (!WS) {
    LBReporter.reportWriteWithoutLogging(VD, C);
  } else if (WS->isLogged()) {
    State = State->set<LogMap>(VD, WriteState::getWritten());
    C.addTransition(State);
  } else if (WS->isWritten()) {
    LBReporter.reportAlreadyWritten(VD, C);
  } else {
    llvm::report_fatal_error("not possible transition");
  }
}
*/
} // namespace clang::ento::nvm