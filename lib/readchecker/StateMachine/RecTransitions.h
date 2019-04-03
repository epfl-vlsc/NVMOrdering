#pragma once
#include "Common.h"
#include "DbgState.h"
#include "RecState.h"

namespace clang::ento::nvm::RecSpace {

void readData(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;
  const char* D = RI.getD();

  const RecState* RS = State->get<RecMap>(D);

  DBG("readData " << (void*)D << " " << RS)

  if (!RS) {
    DBG("!RS bug")
    // bug:not read check
    RI.reportCheckNotRead();
  } else if (RS->isReadCheck()) {
    DBG("isReadCheck bug")
    // read data
    State = State->set<RecMap>(D, RecState::getReadData());
    RI.stateChanged = true;
  } else if (RS->isReadData()) {
    DBG("isReadData bug")
    //do nothing
  } else {
    llvm::report_fatal_error("not possible");
  }
}

void readCheck(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;
  const char* D = RI.getD();

  const RecState* RS = State->get<RecMap>(D);

  DBG("readCheck " << (void*)D << " " << RS)

  if (!RS) {
    DBG("!RS bug")
    // read check
    State = State->set<RecMap>(D, RecState::getReadCheck());
    RI.stateChanged = true;
  } else if (RS->isReadData()) {
    DBG("isReadData bug")
    // do nothing
  }  else if (RS->isReadCheck()) {
    DBG("isReadCheck bug")
    //do nothing
  } else {
    llvm::report_fatal_error("not possible");
  }
}

} // namespace clang::ento::nvm::RecSpace