#pragma once
#include "Common.h"
#include "DbgState.h"
#include "StateInfo.h"
#include "WriteState.h"

namespace clang::ento::nvm::WriteSpace {

void writeData(StateInfo& SI) {
  DBG("writeData")
  ProgramStateRef& State = SI.State;
  const PairInfo* PI = SI.PI;

  const WriteState* WS = State->get<WriteMap>(PI);
  
  if (!WS) {
    DBG("!WS")
    // write data
    State = State->set<WriteMap>(PI, WriteState::getWriteData());
    SI.stateChanged = true;
  } else if (WS->isWriteData()) {
    DBG("isWriteData")
    // correct to write multiple times
  } else if (WS->isFlushData()) {
    DBG("isFlushData bug")
    SI.reportCheckNotCommitted();
  } else if (WS->isPfenceData()) {
    DBG("isPfenceData bug")
    SI.reportCheckNotCommitted();
  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck bug")
    SI.reportCheckNotCommitted();
  } else if (WS->isFlushCheck()) {
    DBG("isFlushCheck bug")
    SI.reportCheckNotCommitted();
  } else if (WS->isPfenceCheck()) {
    DBG("isPfenceCheck")
    // write data
    State = State->set<WriteMap>(PI, WriteState::getWriteData());
    SI.stateChanged = true;
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void flushFenceData(StateInfo& SI) {
  DBG("flushFenceData")
  ProgramStateRef& State = SI.State;
  const PairInfo* PI = SI.PI;

  const WriteState* WS = State->get<WriteMap>(PI);

  if (!WS) {
    DBG("!WS bug")
    SI.reportDataNotWritten();
  } else if (WS->isWriteData()) {
    DBG("isWriteData")
    // flush fence data
    State = State->set<WriteMap>(PI, WriteState::getPfenceData());
    SI.stateChanged = true;
  } else if (WS->isFlushData()) {
    DBG("isFlushData bug")
    SI.reportNotPossible(true);
  } else if (WS->isPfenceData()) {
    DBG("isPfenceData bug")
    SI.reportDataAlreadyCommitted();
  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck bug")
    SI.reportCheckNotCommitted();
  } else if (WS->isFlushCheck()) {
    DBG("isFlushCheck bug")
    SI.reportCheckNotCommitted();
  } else if (WS->isPfenceCheck()) {
    DBG("isPfenceCheck bug")
    SI.reportDataNotWritten();
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void writeCheck(StateInfo& SI) {
  DBG("writeCheck")
  ProgramStateRef& State = SI.State;
  const PairInfo* PI = SI.PI;

  const WriteState* WS = State->get<WriteMap>(PI);

  if (!WS) {
    DBG("!WS bug")
    SI.reportDataNotWritten();
  } else if (WS->isWriteData()) {
    DBG("isWriteData bug")
    SI.reportDataNotCommitted();
  } else if (WS->isFlushData()) {
    DBG("isFlushData bug")
    SI.reportDataNotCommitted();
  } else if (WS->isPfenceData()) {
    DBG("isPfenceData")
    // write check
    State = State->set<WriteMap>(PI, WriteState::getWriteCheck());
    SI.stateChanged = true;
  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck")
    // correct to write multiple times
  } else if (WS->isFlushCheck()) {
    DBG("isFlushCheck bug")
    SI.reportCheckNotCommitted();
  } else if (WS->isPfenceCheck()) {
    DBG("isPfenceCheck bug")
    SI.reportDataNotWritten();
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void flushFenceCheck(StateInfo& SI) {
  DBG("flushFenceCheck")
  ProgramStateRef& State = SI.State;
  const PairInfo* PI = SI.PI;

  const WriteState* WS = State->get<WriteMap>(PI);

  if (!WS) {
    DBG("!WS bug")
    SI.reportDataNotWritten();
  } else if (WS->isWriteData()) {
    DBG("isWriteData bug")
    SI.reportDataNotCommitted();
  } else if (WS->isFlushData()) {
    DBG("isFlushData bug")
    SI.reportDataNotCommitted();
  } else if (WS->isPfenceData()) {
    DBG("isPfenceData bug")
    SI.reportCheckNotWritten();
  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck")
    // write check
    State = State->set<WriteMap>(PI, WriteState::getPfenceCheck());
    SI.stateChanged = true;
  } else if (WS->isFlushCheck()) {
    DBG("isFlushCheck bug")
    SI.reportNotPossible(false);
  } else if (WS->isPfenceCheck()) {
    DBG("isPfenceCheck bug")
    SI.reportCheckAlreadyCommitted();
  } else {
    llvm::report_fatal_error("not possible");
  }
}

} // namespace clang::ento::nvm::WriteSpace
