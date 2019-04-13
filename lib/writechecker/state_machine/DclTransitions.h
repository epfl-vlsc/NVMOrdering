#pragma once
#include "Common.h"
#include "DbgState.h"
#include "DclState.h"
#include "StateInfo.h"

namespace clang::ento::nvm::DclSpace {

void writeData(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  const char* D = SI.getD();

  const DclState* DS = State->get<DclMap>(D);

  DBG("writeData " << (void*)D << " " << DS)

  if (!DS) {
    DBG("!DS bug")
    // write data
    State = State->set<DclMap>(D, DclState::getWriteData());
    SI.stateChanged = true;
  } else if (DS->isWriteData()) {
    DBG("isWriteDatabug ")
    // bug:already written data
    SI.reportDataAlreadyWritten();
  } else if (DS->isFlushData()) {
    DBG("isFlushData bug")
    // bug:already written data
    SI.reportDataAlreadyWritten();
  } else if (DS->isPfenceData()) {
    DBG("isPfenceData bug")
    // bug:already written data
    SI.reportDataAlreadyWritten();
  } else if (DS->isWriteCheck()) {
    DBG("isWriteCheck")
    // write data
    llvm::outs() << "lol\n";
    State = State->set<DclMap>(D, DclState::getWriteData());
    SI.stateChanged = true;
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void flushData(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  const char* D = SI.getD();

  const DclState* DS = State->get<DclMap>(D);

  DBG("flushData " << (void*)D << " " << DS)

  if (!DS) {
    DBG("!DS bug")
    // bug:not written data
    SI.reportDataNotWritten();
  } else if (DS->isWriteData()) {
    DBG("isWriteData")
    // flush data
    State = State->set<DclMap>(D, DclState::getFlushData());
    SI.stateChanged = true;
  } else if (DS->isFlushData()) {
    DBG("isFlushData bug")
    // bug:already flushed data
    SI.reportDataAlreadyFlushed();
  } else if (DS->isPfenceData()) {
    DBG("isPfenceData bug")
    // bug: already flushed data
    SI.reportDataAlreadyFlushed();
  } else if (DS->isWriteCheck()) {
    DBG("isWriteCheck bug")
    // bug: already flushed data
    SI.reportDataAlreadyFlushed();
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void pfenceData(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  const char* D = SI.getD();

  const DclState* DS = State->get<DclMap>(D);

  DBG("pfenceData " << (void*)D << " " << DS)

  if (!DS) {
    DBG("!DS")
    // do nothing
  } else if (DS->isWriteData()) {
    DBG("isWriteData bug")
    // bug:not flushed data
    SI.reportDataNotFlushed();
  } else if (DS->isFlushData()) {
    DBG("isFlushData")
    // pfence data
    State = State->set<DclMap>(D, DclState::getPfenceData());
    SI.stateChanged = true;
  } else if (DS->isPfenceData()) {
    DBG("isPfenceData")
    // do nothing
  } else if (DS->isWriteCheck()) {
    DBG("isWriteCheck")
    // do nothing
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void writeCheck(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  const char* D = SI.getD();

  const DclState* DS = State->get<DclMap>(D);

  DBG("writeCheck " << (void*)D << " " << DS)

  if (!DS) {
    DBG("!DS bug")
    // bug:not written data
    SI.reportDataNotWritten();
  } else if (DS->isWriteData()) {
    DBG("isWriteData bug")
    // bug:not persisted data
    SI.reportDataNotPersisted();
  } else if (DS->isFlushData()) {
    DBG("isFlushData bug")
    // bug:not persisted data
    SI.reportDataNotPersisted();
  } else if (DS->isPfenceData()) {
    DBG("isPfenceData")
    State = State->set<DclMap>(D, DclState::getWriteCheck());
    SI.stateChanged = true;
  } else if (DS->isWriteCheck()) {
    DBG("isWriteCheck bug")
    // bug:already written check
    SI.reportCheckAlreadyWritten();
  } else {
    llvm::report_fatal_error("not possible");
  }
}

} // namespace clang::ento::nvm::DclSpace
