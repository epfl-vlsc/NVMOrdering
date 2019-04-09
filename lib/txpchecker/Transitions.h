#pragma once
#include "Common.h"
#include "DbgState.h"
#include "States.h"
#include "TransitionInfos.h"

namespace clang::ento::nvm::Transitions {

void writeData(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;
  const NamedDecl* Obj = RI.Obj;
  const NamedDecl* Field = RI.Field;

  if (Field) {
    const WriteState* FieldWS = State->get<PMap>(Field);
    const WriteState* ObjWS = State->get<PMap>(Obj);

    if (!FieldWS || FieldWS->isInit() || !ObjWS || ObjWS->isInit()) {
      // if field or obj is not logged
      RI.reportNotLogBeforeWriteBug();
    } else if (FieldWS->isLogged() || ObjWS->isLogged()) {
      // if field or obj is logged
      State = State->set<PMap>(Obj, WriteState::getWritten());
      State = State->set<PMap>(Field, WriteState::getWritten());
      RI.stateChanged = true;
    } else if (FieldWS->isWritten()) {
      // if field is written
      RI.reportDoubleWriteBug();
    } else {
      // do nothing
    }

    return;
  }

  if (Obj) {
    const WriteState* ObjWS = State->get<PMap>(Obj);

    if (!ObjWS || ObjWS->isInit()) {
      // if obj is not logged
      RI.reportNotLogBeforeWriteBug();
    } else if (ObjWS->isLogged()) {
      // if field or obj is logged
      State = State->set<PMap>(Obj, WriteState::getWritten());
      RI.stateChanged = true;
    } else {
      // do nothing
    }
  }
}

void startTx(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;
  unsigned txCount = State->get<TxCounter>();
  txCount += 1;
  State = State->set<TxCounter>(txCount);
  RI.stateChanged = true;
}

void endTx(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;

  unsigned txCount = State->get<TxCounter>();
  txCount -= 1;
  if (txCount < 0) {
    // more tx end
    RI.reportNotTxPairBug();
  } else {
    State = State->set<TxCounter>(txCount);
    RI.stateChanged = true;
  }
}

void pdirectAccess(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;
  const NamedDecl* Obj = RI.Obj;
  if (!Obj) {
    return;
  }

  // if not traced with alloc fncs, trace with pmemdirect
  const WriteState* ObjWS = State->get<PMap>(Obj);

  if (!ObjWS) {
    State = State->set<PMap>(Obj, WriteState::getInit());
    RI.stateChanged = true;
  }
}

void logData(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;
  const NamedDecl* Obj = RI.Obj;
  const NamedDecl* Field = RI.Field;

  // if field is logged, only log field
  // if obj is logged, log whole obj
  if (Field) {
    const WriteState* FieldWS = State->get<PMap>(Field);

    // todo bug report if not inited properly
    if (!FieldWS || FieldWS->isInit() || FieldWS->isWritten()) {
      State = State->set<PMap>(Field, WriteState::getLogged());
      RI.stateChanged = true;
    } else if (FieldWS->isLogged()) {
      RI.reportDoubleLogBugBug();
    } else {
      llvm::report_fatal_error("not possible field state");
    }

    return;
  }

  if (Obj) {
    const WriteState* ObjWS = State->get<PMap>(Obj);

    // todo bug report if not inited properly
    if (!ObjWS || ObjWS->isInit() || ObjWS->isWritten()) {
      State = State->set<PMap>(Obj, WriteState::getLogged());
      RI.stateChanged = true;
    } else if (ObjWS->isLogged()) {
      RI.reportDoubleLogBugBug();
    } else {
      llvm::report_fatal_error("not possible obj state");
    }
  }
}

} // namespace clang::ento::nvm::Transitions
