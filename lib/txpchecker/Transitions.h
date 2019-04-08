#pragma once
#include "Common.h"
#include "DbgState.h"
#include "States.h"
#include "TransitionInfos.h"

namespace clang::ento::nvm {

void writeDirect(ReportInfos& RI) {
  /*
  ProgramStateRef& State = RI.State;
  const ValueDecl* CurrentVD = RI.Current;
  const ValueDecl* BaseVD = RI.Base;

  const WriteState* CurrentWS = State->get<PMap>(CurrentVD);
  const WriteState* BaseWS = State->get<PMap>(BaseVD);

  DBG("writeDirect " << (void*)CurrentVD << " " << CurrentWS)

  if (CurrentWS) {
    if (CurrentWS->isLogged()) {
      DBG("isLogged")
      State = State->set<PMap>(CurrentVD, WriteState::getWritten());
      RI.stateChanged = true;
    } else if (CurrentWS->isWritten()) {
      DBG("isWritten bug")
      // RI.reportDoubleWriteBug();
    } else {
      if (BaseWS && BaseWS->isLogged()) {
        DBG("base isLogged")
        State = State->set<PMap>(CurrentVD, WriteState::getWritten());
        RI.stateChanged = true;
      } else {
        // RI.reportNotLogBeforeWriteBug();
      }
    }
  } else {
    if (BaseWS && BaseWS->isLogged()) {
      DBG("base isLogged")
      State = State->set<PMap>(CurrentVD, WriteState::getWritten());
      RI.stateChanged = true;
    } else {
      // RI.reportNotLogBeforeWriteBug();
    }
  }
  */
}

} // namespace clang::ento::nvm
