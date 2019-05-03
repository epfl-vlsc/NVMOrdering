#pragma once
#include "Common.h"

namespace clang::ento::nvm::DclSpace {

void writeData(StateInfo& SI) {
  DBG("writeData")
  ProgramStateRef& State = SI.State;
  const PairInfo* PI = SI.PI;

  const WriteState* WS = State->get<WriteMap>(PI);

  if (!WS) {
    DBG("!WS")

  } else if (WS->isWriteData()) {
    DBG("isWriteData")

  } else if (WS->isFlushData()) {
    DBG("isFlushData")

  } else if (WS->isPfenceData()) {
    DBG("isPfenceData")

  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck")

  } else if (WS->isFlushCheck()) {
    DBG("isFlushCheck")

  } else if (WS->isPfenceCheck()) {
    DBG("isPfenceCheck")

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
    DBG("!WS")

  } else if (WS->isWriteData()) {
    DBG("isWriteData")

  } else if (WS->isFlushData()) {
    DBG("isFlushData")

  } else if (WS->isPfenceData()) {
    DBG("isPfenceData")

  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck")

  } else if (WS->isFlushCheck()) {
    DBG("isFlushCheck")

  } else if (WS->isPfenceCheck()) {
    DBG("isPfenceCheck")

  } else {
    llvm::report_fatal_error("not possible");
  }
}

void flushData(StateInfo& SI) {
  DBG("flushData")
  ProgramStateRef& State = SI.State;
  const PairInfo* PI = SI.PI;

  const WriteState* WS = State->get<WriteMap>(PI);

  if (!WS) {
    DBG("!WS")

  } else if (WS->isWriteData()) {
    DBG("isWriteData")

  } else if (WS->isFlushData()) {
    DBG("isFlushData")

  } else if (WS->isPfenceData()) {
    DBG("isPfenceData")

  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck")

  } else if (WS->isFlushCheck()) {
    DBG("isFlushCheck")

  } else if (WS->isPfenceCheck()) {
    DBG("isPfenceCheck")

  } else {
    llvm::report_fatal_error("not possible");
  }
}

void flushCheck(StateInfo& SI) {
  DBG("flushCheck")
  ProgramStateRef& State = SI.State;
  const PairInfo* PI = SI.PI;

  const WriteState* WS = State->get<WriteMap>(PI);

  if (!WS) {
    DBG("!WS")

  } else if (WS->isWriteData()) {
    DBG("isWriteData")

  } else if (WS->isFlushData()) {
    DBG("isFlushData")

  } else if (WS->isPfenceData()) {
    DBG("isPfenceData")

  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck")

  } else if (WS->isFlushCheck()) {
    DBG("isFlushCheck")

  } else if (WS->isPfenceCheck()) {
    DBG("isPfenceCheck")

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

  } else if (WS->isWriteData()) {
    DBG("isWriteData")

  } else if (WS->isFlushData()) {
    DBG("isFlushData")

  } else if (WS->isPfenceData()) {
    DBG("isPfenceData")

  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck")

  } else if (WS->isFlushCheck()) {
    DBG("isFlushCheck")

  } else if (WS->isPfenceCheck()) {
    DBG("isPfenceCheck")

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

  } else if (WS->isWriteData()) {
    DBG("isWriteData")

  } else if (WS->isFlushData()) {
    DBG("isFlushData")

  } else if (WS->isPfenceData()) {
    DBG("isPfenceData")

  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck")

  } else if (WS->isFlushCheck()) {
    DBG("isFlushCheck")

  } else if (WS->isPfenceCheck()) {
    DBG("isPfenceCheck")

  } else {
    llvm::report_fatal_error("not possible");
  }
}

void pfence(StateInfo& SI) {
  DBG("pfence")
  ProgramStateRef& State = SI.State;
  const PairInfo* PI = SI.PI;

  const WriteState* WS = State->get<WriteMap>(PI);

  if (!WS) {
    DBG("!WS")

  } else if (WS->isWriteData()) {
    DBG("isWriteData")

  } else if (WS->isFlushData()) {
    DBG("isFlushData")

  } else if (WS->isPfenceData()) {
    DBG("isPfenceData")

  } else if (WS->isWriteCheck()) {
    DBG("isWriteCheck")

  } else if (WS->isFlushCheck()) {
    DBG("isFlushCheck")

  } else if (WS->isPfenceCheck()) {
    DBG("isPfenceCheck")

  } else {
    llvm::report_fatal_error("not possible");
  }
}

} // namespace clang::ento::nvm::DclSpace
