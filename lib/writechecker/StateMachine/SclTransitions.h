#pragma once
#include "Common.h"
#include "SclState.h"

namespace clang::ento::nvm::SclSpace {
void writeData(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;
  const char* D = RI.getD();

  const SclState* SS = State->get<SclMap>(D);

  if (!SS) {
    State = State->set<SclMap>(D, SclState::getWriteData());
    RI.stateChanged = true;
  } else if (SS->isWriteData()) {
    // bug:already written data
    RI.reportDataAlreadyWritten();
  } else if (SS->isVfenceData()) {
    // bug:already written data
    RI.reportDataAlreadyWritten();
  } else if (SS->isWriteCheck()) {
    State = State->set<SclMap>(D, SclState::getWriteData());
    RI.stateChanged = true;
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void vfenceData(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;
  const char* D = RI.getD();

  const SclState* SS = State->get<SclMap>(D);


  if (!SS) {
    //do nothing
  } else if (SS->isWriteData()) {
    State = State->set<SclMap>(D, SclState::getVfenceData());
    RI.stateChanged = true;
  } else if (SS->isVfenceData()) {
    //do nothing
  } else if (SS->isWriteCheck()) {
    //do nothing
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void writeCheck(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;
  const char* D = RI.getD();

  const SclState* SS = State->get<SclMap>(D);

  if (!SS) {
    // bug:not written data
    RI.reportDataNotWritten();
  } else if (SS->isWriteData()) {
    // bug:not fenced data
    RI.reportDataNotFenced();
  } else if (SS->isVfenceData()) {
    State = State->set<SclMap>(D, SclState::getWriteCheck());
    RI.stateChanged = true;
  } else if (SS->isWriteCheck()) {
    // bug:already written check
    RI.reportCheckAlreadyWritten();
  } else {
    llvm::report_fatal_error("not possible");
  }
}

} // namespace clang::ento::nvm::SclSpace
