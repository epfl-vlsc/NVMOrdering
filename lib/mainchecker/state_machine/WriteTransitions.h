#pragma once
#include "Common.h"
#include "DbgState.h"
#include "WriteState.h"
#include "StateInfo.h"

namespace clang::ento::nvm::WriteSpace {
/*
void writeData(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  const NamedDecl* ND = SI.dataND;

  const WriteState* WS = State->get<WriteMap>(ND);

  DBG("writeData " << ND << " " << WS)

  if (!WS) {
    DBG("!WS bug")
    // write data
    State = State->set<WriteMap>(ND, WriteState::getWriteData());
    SI.stateChanged = true;
  } else if (WS->isWriteData()) {
    DBG("isWriteDatabug ")
    // bug:already written data
    SI.reportDataAlreadyWritten();
  } else if (WS->isFlushData()) {
    DBG("isFlushData bug")
    // bug:already written data
    SI.reportDataAlreadyWritten();
  } else if (WS->isPfenceData()) {
    DBG("isPfenceData bug")
    // bug:already written data
    SI.reportDataAlreadyWritten();
  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck")
    // write data
    llvm::outs() << "lol\n";
    State = State->set<WriteMap>(ND, WriteState::getWriteData());
    SI.stateChanged = true;
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void flushFenceData(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  const NamedDecl* ND = SI.dataND;

  const WriteState* WS = State->get<WriteMap>(ND);

  DBG("flushData " << ND << " " << WS)

  if (!WS) {
    DBG("!WS bug")
    // bug:not written data
    SI.reportDataNotWritten();
  } else if (WS->isWriteData()) {
    DBG("isWriteData")
    // flush data
    State = State->set<WriteMap>(ND, WriteState::getFlushData());
    SI.stateChanged = true;
  } else if (WS->isFlushData()) {
    DBG("isFlushData bug")
    // bug:already flushed data
  } else if (WS->isPfenceData()) {
    DBG("isPfenceData bug")
    // bug: already flushed data
  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck bug")
    // bug: already flushed data

  } else {
    llvm::report_fatal_error("not possible");
  }
}

void writeCheck(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  const NamedDecl* ND = SI.checkND;

  const WriteState* WS = State->get<WriteMap>(ND);

  DBG("writeCheck " << ND << " " << WS)

  if (!WS) {
    DBG("!WS bug")
    // bug:not written data
    SI.reportDataNotWritten();
  } else if (WS->isWriteData()) {
    DBG("isWriteData bug")
    // bug:not persisted data
    SI.reportDataNotPersisted();
  } else if (WS->isFlushData()) {
    DBG("isFlushData bug")
    // bug:not persisted data
    SI.reportDataNotPersisted();
  } else if (WS->isPfenceData()) {
    DBG("isPfenceData")
    State = State->set<WriteMap>(ND, WriteState::getWriteCheck());
    SI.stateChanged = true;
  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck bug")
    // bug:already written check
    SI.reportCheckAlreadyWritten();
  } else {
    llvm::report_fatal_error("not possible");
  }
}
*/
} // namespace clang::ento::nvm::WriteSpace
