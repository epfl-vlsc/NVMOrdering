#pragma once
#include "Common.h"

namespace clang::ento::nvm::SclSpace {

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

void fence(StateInfo& SI) {
  DBG("fence")
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

} // namespace clang::ento::nvm::SclSpace
