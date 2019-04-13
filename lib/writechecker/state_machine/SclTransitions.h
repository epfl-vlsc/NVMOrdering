#pragma once
#include "Common.h"
#include "SclState.h"
#include "StateInfo.h"

namespace clang::ento::nvm::SclSpace {
void writeData(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  const char* D = SI.getD();

  const SclState* SS = State->get<SclMap>(D);

  if (!SS) {
    State = State->set<SclMap>(D, SclState::getWriteData());
    SI.stateChanged = true;
  } else if (SS->isWriteData()) {
    if (!SI.useMask()) {
      // bug:already written data
      SI.reportDataAlreadyWritten();
    } else {
      // todo do nothing uses mask
    }
  } else if (SS->isVfenceData()) {
    // bug:already written data
    SI.reportDataAlreadyWritten();
  } else if (SS->isWriteCheck()) {
    State = State->set<SclMap>(D, SclState::getWriteData());
    SI.stateChanged = true;
  } else {
    llvm::report_fatal_error("not possible");
  }
}
void vfenceData(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  const char* D = SI.getD();

  const SclState* SS = State->get<SclMap>(D);

  if (!SS) {
    // do nothing
  } else if (SS->isWriteData()) {
    State = State->set<SclMap>(D, SclState::getVfenceData());
    SI.stateChanged = true;
  } else if (SS->isVfenceData()) {
    // do nothing
  } else if (SS->isWriteCheck()) {
    // do nothing
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void writeCheck(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  const char* D = SI.getD();

  const SclState* SS = State->get<SclMap>(D);

  if (!SS) {
    // bug:not written data
    SI.reportDataNotWritten();
  } else if (SS->isWriteData()) {
    // bug:not fenced data
    SI.reportDataNotFenced();
  } else if (SS->isVfenceData()) {
    State = State->set<SclMap>(D, SclState::getWriteCheck());
    SI.stateChanged = true;
  } else if (SS->isWriteCheck()) {
    if (!SI.useMask()) {
      // bug:already written check
      SI.reportCheckAlreadyWritten();
    } else {
      // todo do nothing uses mask
    }
  } else {
    llvm::report_fatal_error("not possible");
  }
}

} // namespace clang::ento::nvm::SclSpace
