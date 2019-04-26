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

    DBG("writeData field" << (void*)Field << " " << FieldWS)
    DBG("writeData obj" << (void*)Obj << " " << ObjWS)

    DBG("field:" <<Field->getNameAsString())
    DBG("obj:" << Obj->getNameAsString())

    if (!FieldWS || FieldWS->isInit() || !ObjWS || ObjWS->isInit()) {
      DBG("!FieldWS or !ObjWS")
      // if field or obj is not logged
      RI.reportNotLogBeforeWriteBug();
    } else if (FieldWS->isLogged() || ObjWS->isLogged()) {
      DBG("isLogged")
      // if field or obj is logged
      State = State->set<PMap>(Obj, WriteState::getWritten());
      State = State->set<PMap>(Field, WriteState::getWritten());
      RI.stateChanged = true;
    } else if (FieldWS->isWritten()) {
      DBG("isWritten")
      // if field is written
      RI.reportDoubleWriteBug();
    } else {
      DBG("other")
      // do nothing
    }

    return;
  }

  if (Obj) {
    //todo seperate alloc from data writes

    const WriteState* ObjWS = State->get<PMap>(Obj);

    DBG("writeData obj" << (void*)Obj << " " << ObjWS)
    DBG("obj:" <<Obj->getNameAsString())

    if (!ObjWS || ObjWS->isInit()) {
      DBG("!ObjWS")
      // if obj is not logged
      RI.reportNotLogBeforeWriteBug();
    } else if (ObjWS->isLogged()) {
      DBG("isLogged")
      // if field or obj is logged
      State = State->set<PMap>(Obj, WriteState::getLogged());
      RI.stateChanged = true;
    } else {
      DBG("other")
      // do nothing
    }
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

  DBG("pdirectAccess obj" << (void*)Obj << " " << ObjWS)

  if (!ObjWS) {
    DBG("!ObjWS")
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

    DBG("logData field" << (void*)Field << " " << FieldWS)
    DBG("field:" <<Field->getNameAsString())

    // todo bug report if not inited properly
    if (!FieldWS || FieldWS->isInit() || FieldWS->isWritten()) {
      DBG("any")
      State = State->set<PMap>(Field, WriteState::getLogged());
      RI.stateChanged = true;
    } else if (FieldWS->isLogged()) {
      DBG("isLogged")
      RI.reportDoubleLogBugBug();
    } else {
      llvm::report_fatal_error("not possible field state");
    }

    return;
  }

  if (Obj) {
    const WriteState* ObjWS = State->get<PMap>(Obj);

    DBG("logData obj" << (void*)Obj << " " << ObjWS)
    DBG("obj:" <<Obj->getNameAsString())

    // todo bug report if not inited properly
    if (!ObjWS || ObjWS->isInit() || ObjWS->isWritten()) {
      DBG("any")
      State = State->set<PMap>(Obj, WriteState::getLogged());
      RI.stateChanged = true;
    } else if (ObjWS->isLogged()) {
      DBG("isLogged")
      RI.reportDoubleLogBugBug();
    } else {
      llvm::report_fatal_error("not possible obj state");
    }
  }
}

} // namespace clang::ento::nvm::Transitions
