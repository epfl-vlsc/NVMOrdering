#pragma once
#include "Common.h"
#include "StateInfo.h"
namespace clang::ento::nvm::SclSpace {

void writeData(StateInfo& SI) {
  DBG("writeData")
  ProgramStateRef& State = SI.State;
  const PairInfo* PI = SI.PI;

  const WriteState* WS = State->get<WriteMap>(PI);

  if (!WS) {
    DBG("!WS")
    SI.doTransition(WriteState::KindWD());
  } else if (WS->isWriteData()) {
    DBG("isWriteData")

  } else if (WS->isPfenceData()) {
    DBG("isPfenceData")
    SI.doTransition(WriteState::KindWD());
  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck")
    SI.reportCommitBug(false);
  } else if (WS->isPfenceCheck()) {
    DBG("isPfenceCheck")
    SI.doTransition(WriteState::KindWD());
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void writeCheck(StateInfo& SI) {
  DBG("writeData")
  ProgramStateRef& State = SI.State;
  const PairInfo* PI = SI.PI;

  const WriteState* WS = State->get<WriteMap>(PI);

  if (!WS) {
    DBG("!WS")
    SI.doTransition(WriteState::KindWC());
  } else if (WS->isWriteData()) {
    DBG("isWriteData")
    SI.reportCommitBug(true);
  } else if (WS->isPfenceData()) {
    DBG("isPfenceData")
    SI.doTransition(WriteState::KindWC());
  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck")

  } else if (WS->isPfenceCheck()) {
    DBG("isPfenceCheck")
    SI.doTransition(WriteState::KindWC());
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
    DBG("!WS")
    SI.reportWriteBug(true);
  } else if (WS->isWriteData()) {
    DBG("isWriteData")
    SI.doTransition(WriteState::KindPD());
  } else if (WS->isPfenceData()) {
    DBG("isPfenceData")
    SI.reportDoubleFlushBug(true);
  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck")
    SI.reportCommitBug(false);
  } else if (WS->isPfenceCheck()) {
    DBG("isPfenceCheck")
    SI.reportWriteBug(true);
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
    DBG("!WS")
    SI.reportWriteBug(false);
  } else if (WS->isWriteData()) {
    DBG("isWriteData")
    SI.reportCommitBug(true);
  } else if (WS->isPfenceData()) {
    DBG("isPfenceData")
    SI.reportWriteBug(false);
  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck")
    SI.doTransition(WriteState::KindPC());
  } else if (WS->isPfenceCheck()) {
    DBG("isPfenceCheck")
    SI.reportDoubleFlushBug(false);
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void fence(StateInfo& SI) {
  DBG("fence")
  ProgramStateRef& State = SI.State;
  const PairInfo* PI = SI.PI;

  const WriteState* WS = State->get<WriteMap>(PI);

  if (!WS) {
    DBG("!WS")

  } else if (WS->isWriteData()) {
    DBG("isWriteData")
    SI.doTransition(WriteState::KindPD());
  } else if (WS->isPfenceData()) {
    DBG("isPfenceData")

  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck")
    SI.doTransition(WriteState::KindPC());
  } else if (WS->isPfenceCheck()) {
    DBG("isPfenceCheck")

  } else {
    llvm::report_fatal_error("not possible");
  }
}

} // namespace clang::ento::nvm::SclSpace
