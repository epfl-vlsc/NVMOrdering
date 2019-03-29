#pragma once
#include "Common.h"
#include "States/DclState.h"

namespace clang::ento::nvm::dcl {

bool writeData(ProgramStateRef& State, char* D) {
  const DclState* DS = State->get<DclMap>(D);

  if (!DS) {
    State = State->set<DclMap>(D, DclState::getWriteData());
  } else if (DS->isWriteData()) {
    // bug:already written data
  } else if (DS->isFlushData()) {
    // bug:already written data
  } else if (DS->isPfenceData()) {
    // bug:already written data
  } else if (DS->isWriteCheck()) {
    State = State->set<DclMap>(D, DclState::getWriteData());
    return true;
  } else {
    llvm::report_fatal_error("not possible");
  }

  return false;
}

bool flushData(ProgramStateRef& State, char* D) {
  const DclState* DS = State->get<DclMap>(D);

  if (!DS) {
    // bug:not written data
  } else if (DS->isWriteData()) {
    State = State->set<DclMap>(D, DclState::getFlushData());
    return true;
  } else if (DS->isFlushData()) {
    // bug:already flushed data
  } else if (DS->isPfenceData()) {
    // bug: already flushed data
  } else if (DS->isWriteCheck()) {
    // bug: already flushed data
  } else {
    llvm::report_fatal_error("not possible");
  }

  return false;
}

bool pfenceData(ProgramStateRef& State, char* D) {
  const DclState* DS = State->get<DclMap>(D);

  if (!DS) {
    return false;
  } else if (DS->isWriteData()) {
    // bug:not flushed data
  } else if (DS->isFlushData()) {
    State = State->set<DclMap>(D, DclState::getPfenceData());
    return true;
  } else if (DS->isPfenceData()) {
    return false;
  } else if (DS->isWriteCheck()) {
    return false;
  } else {
    llvm::report_fatal_error("not possible");
  }

  return false;
}

bool writeCheck(ProgramStateRef& State, char* D) {
  const DclState* DS = State->get<DclMap>(D);

  if (!DS) {
    // bug:not written data
  } else if (DS->isWriteData()) {
    // bug:not persisted data
  } else if (DS->isFlushData()) {
    // bug:not persisted data
  } else if (DS->isPfenceData()) {
    State = State->set<DclMap>(D, DclState::getWriteCheck());
    return true;
  } else if (DS->isWriteCheck()) {
    // bug:already written check
  } else {
    llvm::report_fatal_error("not possible");
  }

  return false;
}


} // namespace clang::ento::nvm::dcl
