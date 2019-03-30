#pragma once
#include "Common.h"
#include "DclState.h"
#include "DbgState.h"

namespace clang::ento::nvm::DclSpace {

void writeData(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;
  const char* D = RI.getD();

  const DclState* DS = State->get<DclMap>(D);

  DBG("writeData " << (void*)D << " " << DS)

  if (!DS) {
    DBG("!DS bug")
    //write data
    State = State->set<DclMap>(D, DclState::getWriteData());
    RI.stateChanged = true;
  } else if (DS->isWriteData()) {
    DBG("isWriteDatabug ")
    // bug:already written data
    RI.reportDataAlreadyWritten();
  } else if (DS->isFlushData()) {
    DBG("isFlushData bug")
    // bug:already written data
    RI.reportDataAlreadyWritten();
  } else if (DS->isPfenceData()) {
    DBG("isPfenceData bug")
    // bug:already written data
    RI.reportDataAlreadyWritten();
  } else if (DS->isWriteCheck()) {
    DBG("isWriteCheck")
    //write data
    State = State->set<DclMap>(D, DclState::getWriteData());
    RI.stateChanged = true;
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void flushData(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;
  const char* D = RI.getD();

  const DclState* DS = State->get<DclMap>(D);

  DBG("flushData " << (void*)D << " " << DS)

  if (!DS) {
    DBG("!DS bug")
    // bug:not written data
    RI.reportDataNotWritten();
  } else if (DS->isWriteData()) {
    DBG("isWriteData")
    //flush data
    State = State->set<DclMap>(D, DclState::getFlushData());
    RI.stateChanged = true;
  } else if (DS->isFlushData()) {
    DBG("isFlushData bug")
    // bug:already flushed data
    RI.reportDataAlreadyFlushed();
  } else if (DS->isPfenceData()) {
    DBG("isPfenceData bug")
    // bug: already flushed data
    RI.reportDataAlreadyFlushed();
  } else if (DS->isWriteCheck()) {
    DBG("isWriteCheck bug")
    // bug: already flushed data
    RI.reportDataAlreadyFlushed();
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void pfenceData(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;
  const char* D = RI.getD();

  const DclState* DS = State->get<DclMap>(D);

  DBG("pfenceData " << (void*)D << " " << DS)

  if (!DS) {
    DBG("!DS")
    //do nothing
  } else if (DS->isWriteData()) {
    DBG("isWriteData bug")
    // bug:not flushed data
    RI.reportDataNotFlushed();
  } else if (DS->isFlushData()) {
    DBG("isFlushData")
    //pfence data
    State = State->set<DclMap>(D, DclState::getPfenceData());
    RI.stateChanged = true;
  } else if (DS->isPfenceData()) {
    DBG("isPfenceData")
    //do nothing
  } else if (DS->isWriteCheck()) {
    DBG("isWriteCheck")
    //do nothing
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void writeCheck(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;
  const char* D = RI.getD();

  const DclState* DS = State->get<DclMap>(D);

  DBG("writeCheck " << (void*)D << " " << DS)

  if (!DS) {
    DBG("!DS bug")
    // bug:not written data
    RI.reportDataNotWritten();
  } else if (DS->isWriteData()) {
    DBG("isWriteData bug")
    // bug:not persisted data
    RI.reportDataNotPersisted();
  } else if (DS->isFlushData()) {
    DBG("isFlushData bug")
    // bug:not persisted data
    RI.reportDataNotPersisted();
  } else if (DS->isPfenceData()) {
    DBG("isPfenceData")
    State = State->set<DclMap>(D, DclState::getWriteCheck());
    RI.stateChanged = true;
  } else if (DS->isWriteCheck()) {
    DBG("isWriteCheck bug")
    // bug:already written check
    RI.reportCheckAlreadyWritten();
  } else {
    llvm::report_fatal_error("not possible");
  }
}


} // namespace clang::ento::nvm::dcl
