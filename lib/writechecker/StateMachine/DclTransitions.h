#pragma once
#include "Common.h"
#include "DclState.h"

namespace clang::ento::nvm::DclSpace {

void writeData(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;
  const char* D = RI.getD();

  const DclState* DS = State->get<DclMap>(D);

  if (!DS) {
    //write data
    State = State->set<DclMap>(D, DclState::getWriteData());
    RI.stateChanged = true;
  } else if (DS->isWriteData()) {
    // bug:already written data
    RI.reportDataAlreadyWritten();
  } else if (DS->isFlushData()) {
    // bug:already written data
    RI.reportDataAlreadyWritten();
  } else if (DS->isPfenceData()) {
    // bug:already written data
    RI.reportDataAlreadyWritten();
  } else if (DS->isWriteCheck()) {
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

  if (!DS) {
    // bug:not written data
    RI.reportDataNotWritten();
  } else if (DS->isWriteData()) {
    State = State->set<DclMap>(D, DclState::getFlushData());
    RI.stateChanged = true;
  } else if (DS->isFlushData()) {
    // bug:already flushed data
    RI.reportDataAlreadyFlushed();
  } else if (DS->isPfenceData()) {
    // bug: already flushed data
    RI.reportDataAlreadyFlushed();
  } else if (DS->isWriteCheck()) {
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

  if (!DS) {
    //do nothing
  } else if (DS->isWriteData()) {
    // bug:not flushed data
    RI.reportDataNotFlushed();
  } else if (DS->isFlushData()) {
    State = State->set<DclMap>(D, DclState::getPfenceData());
    RI.stateChanged = true;
  } else if (DS->isPfenceData()) {
    //do nothing
  } else if (DS->isWriteCheck()) {
    //do nothing
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void writeCheck(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;
  const char* D = RI.getD();

  const DclState* DS = State->get<DclMap>(D);

  if (!DS) {
    // bug:not written data
    RI.reportDataNotWritten();
  } else if (DS->isWriteData()) {
    // bug:not persisted data
    RI.reportDataNotPersisted();
  } else if (DS->isFlushData()) {
    // bug:not persisted data
    RI.reportDataNotPersisted();
  } else if (DS->isPfenceData()) {
    State = State->set<DclMap>(D, DclState::getWriteCheck());
    RI.stateChanged = true;
  } else if (DS->isWriteCheck()) {
    // bug:already written check
    RI.reportCheckAlreadyWritten();
  } else {
    llvm::report_fatal_error("not possible");
  }
}


} // namespace clang::ento::nvm::dcl
