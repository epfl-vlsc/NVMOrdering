#pragma once
#include "CheckState.h"
#include "Common.h"
#include "StateInfo.h"

namespace clang::ento::nvm::CheckSpace {

void writeData(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  const char* D = SI.getD();

  const CheckState* CS = State->get<CheckMap>(D);

  if (!CS) {
    // write data
    State = State->set<CheckMap>(D, CheckState::getWriteData());
    SI.stateChanged = true;
  } else if (CS->isWriteData()) {
    // bug:already written data
    SI.reportDataAlreadyWritten();
  } else if (CS->isFlushData()) {
    // bug:already written data
    SI.reportDataAlreadyWritten();
  } else if (CS->isPfenceData()) {
    // write data
    State = State->set<CheckMap>(D, CheckState::getWriteData());
    SI.stateChanged = true;
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void flushData(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  const char* D = SI.getD();

  const CheckState* CS = State->get<CheckMap>(D);

  if (!CS) {
    // bug:not written data
    SI.reportDataNotWritten();
  } else if (CS->isWriteData()) {
    // flush data
    State = State->set<CheckMap>(D, CheckState::getFlushData());
    SI.stateChanged = true;
  } else if (CS->isFlushData()) {
    // bug:already flushed
    SI.reportDataAlreadyFlushed();
  } else if (CS->isPfenceData()) {
    // bug: already flushed
    SI.reportDataAlreadyFlushed();
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void pfenceData(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  const char* D = SI.getD();

  const CheckState* CS = State->get<CheckMap>(D);

  if (!CS) {
    // do nothing
  } else if (CS->isWriteData()) {
    // bug:not flushed
    SI.reportDataNotFlushed();
  } else if (CS->isFlushData()) {
    // pfence data
    State = State->set<CheckMap>(D, CheckState::getPfenceData());
    SI.stateChanged = true;
  } else if (CS->isPfenceData()) {
    // do nothing
  } else {
    llvm::report_fatal_error("not possible");
  }
}

} // namespace clang::ento::nvm::CheckSpace
