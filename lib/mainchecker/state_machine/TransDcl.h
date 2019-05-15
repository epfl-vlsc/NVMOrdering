#pragma once
#include "Common.h"
#include "StateInfo.h"
namespace clang::ento::nvm::DclSpace {

void writeData(StateInfo& SI) {
  DBG("trans writeData")
  ProgramStateRef& State = SI.State;
  const PairInfo* PI = SI.PI;

  const WriteState* WS = State->get<WriteMap>(PI);

  if (!WS) {
    DBG("WS -> WriteData")
    SI.doTransition(WriteState::KindWD());
  } else if (WS->isWriteData()) {
    DBG("WriteData")

  } else if (WS->isFlushData()) {
    DBG("FlushData -> WriteData")
    SI.doTransition(WriteState::KindWD());
  } else if (WS->isPfenceData()) {
    DBG("PfenceData -> WriteData")
    SI.doTransition(WriteState::KindWD());
  } else if (WS->isWriteCheck()) {
    DBG("WriteCheck")
    SI.reportCommitBug(false);
  } else if (WS->isFlushCheck()) {
    DBG("FlushCheck")
    SI.reportCommitBug(false);
  } else if (WS->isPfenceCheck()) {
    DBG("PfenceCheck -> WriteData")
    SI.doTransition(WriteState::KindWD());
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void writeCheck(StateInfo& SI) {
  DBG("trans writeCheck")
  ProgramStateRef& State = SI.State;
  const PairInfo* PI = SI.PI;

  const WriteState* WS = State->get<WriteMap>(PI);

  if (!WS) {
    DBG("WS -> WriteCheck")
    SI.doTransition(WriteState::KindWC());
  } else if (WS->isWriteData()) {
    DBG("WriteData")
    SI.reportCommitBug(true);
  } else if (WS->isFlushData()) {
    DBG("FlushData")
    SI.reportCommitBug(true);
  } else if (WS->isPfenceData()) {
    DBG("PfenceData -> WriteCheck")
    SI.doTransition(WriteState::KindWC());
  } else if (WS->isWriteCheck()) {
    DBG("WriteCheck")

  } else if (WS->isFlushCheck()) {
    DBG("FlushCheck -> WriteCheck")
    SI.doTransition(WriteState::KindWC());
  } else if (WS->isPfenceCheck()) {
    DBG("PfenceCheck -> WriteCheck")
    SI.doTransition(WriteState::KindWC());
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void flushData(StateInfo& SI) {
  DBG("trans flushData")
  ProgramStateRef& State = SI.State;
  const PairInfo* PI = SI.PI;

  const WriteState* WS = State->get<WriteMap>(PI);

  if (!WS) {
    DBG("WS")
    SI.reportWriteBug(true);
  } else if (WS->isWriteData()) {
    DBG("WriteData -> Flush Data")
    SI.doTransition(WriteState::KindFD());
  } else if (WS->isFlushData()) {
    DBG("FlushData")
    SI.reportDoubleFlushBug(true);
  } else if (WS->isPfenceData()) {
    DBG("PfenceData")
    SI.reportWriteBug(true);
  } else if (WS->isWriteCheck()) {
    DBG("WriteCheck")
    SI.reportCommitBug(false);
  } else if (WS->isFlushCheck()) {
    DBG("FlushCheck")
    SI.reportCommitBug(false);
  } else if (WS->isPfenceCheck()) {
    DBG("PfenceCheck")
    SI.reportWriteBug(true);
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void flushCheck(StateInfo& SI) {
  DBG("trans flushCheck")
  ProgramStateRef& State = SI.State;
  const PairInfo* PI = SI.PI;

  const WriteState* WS = State->get<WriteMap>(PI);

  if (!WS) {
    DBG("WS")
    SI.reportWriteBug(false);
  } else if (WS->isWriteData()) {
    DBG("WriteData")
    SI.reportCommitBug(true);
  } else if (WS->isFlushData()) {
    DBG("FlushData")
    SI.reportCommitBug(true);
  } else if (WS->isPfenceData()) {
    DBG("PfenceData")
    SI.reportWriteBug(false);
  } else if (WS->isWriteCheck()) {
    DBG("WriteCheck -> FlushCheck")
    SI.doTransition(WriteState::KindFC());
  } else if (WS->isFlushCheck()) {
    DBG("FlushCheck")
    SI.reportDoubleFlushBug(false);
  } else if (WS->isPfenceCheck()) {
    DBG("PfenceCheck")
    SI.reportWriteBug(false);
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void flushFenceData(StateInfo& SI) {
  DBG("trans flushFenceData")
  ProgramStateRef& State = SI.State;
  const PairInfo* PI = SI.PI;

  const WriteState* WS = State->get<WriteMap>(PI);

  if (!WS) {
    DBG("WS")
    SI.reportWriteBug(true);
  } else if (WS->isWriteData()) {
    DBG("WriteData -> PfenceData")
    SI.doTransition(WriteState::KindPD());
  } else if (WS->isFlushData()) {
    DBG("FlushData")
    //SI.reportDoubleFlushBug(true);
  } else if (WS->isPfenceData()) {
    DBG("PfenceData")
    //SI.reportDoubleFlushBug(true);
  } else if (WS->isWriteCheck()) {
    DBG("WriteCheck")
    SI.reportCommitBug(false);
  } else if (WS->isFlushCheck()) {
    DBG("FlushCheck")
    SI.reportCommitBug(false);
  } else if (WS->isPfenceCheck()) {
    DBG("PfenceCheck")
    SI.reportWriteBug(true);
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void flushFenceCheck(StateInfo& SI) {
  DBG("trans flushFenceCheck")
  ProgramStateRef& State = SI.State;
  const PairInfo* PI = SI.PI;

  const WriteState* WS = State->get<WriteMap>(PI);

  if (!WS) {
    DBG("WS")
    SI.reportWriteBug(false);
  } else if (WS->isWriteData()) {
    DBG("WriteData")
    SI.reportCommitBug(true);
  } else if (WS->isFlushData()) {
    DBG("FlushData")
    SI.reportCommitBug(true);
  } else if (WS->isPfenceData()) {
    DBG("PfenceData")
    SI.reportWriteBug(false);
  } else if (WS->isWriteCheck()) {
    DBG("WriteCheck -> PfenceCheck")
    SI.doTransition(WriteState::KindPC());
  } else if (WS->isFlushCheck()) {
    DBG("FlushCheck")
    //SI.reportDoubleFlushBug(false);
  } else if (WS->isPfenceCheck()) {
    DBG("PfenceCheck")
    //SI.reportWriteBug(false);
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void pfence(StateInfo& SI) {
  DBG("trans pfence")
  ProgramStateRef& State = SI.State;
  const PairInfo* PI = SI.PI;

  const WriteState* WS = State->get<WriteMap>(PI);

  if (!WS) {
    DBG("WS")

  } else if (WS->isWriteData()) {
    DBG("WriteData")
    
  } else if (WS->isFlushData()) {
    DBG("FlushData -> PfenceData")
    SI.doTransition(WriteState::KindPD());
  } else if (WS->isPfenceData()) {
    DBG("PfenceData")

  } else if (WS->isWriteCheck()) {
    DBG("WriteCheck")
    
  } else if (WS->isFlushCheck()) {
    DBG("FlushCheck -> PfenceCheck")
    SI.doTransition(WriteState::KindPC());
  } else if (WS->isPfenceCheck()) {
    DBG("PfenceCheck")

  } else {
    llvm::report_fatal_error("not possible");
  }
}

} // namespace clang::ento::nvm::DclSpace
