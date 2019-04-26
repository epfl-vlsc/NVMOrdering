#pragma once
#include "Common.h"
#include "DbgState.h"
#include "StateInfo.h"
#include "States.h"

namespace clang::ento::nvm::WriteSpace {

// todo handle writeobj

void writeObj(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  if (!SI.inTx) {
    DBG("not in tx")
    SI.reportAccessOutTxBug();
    return;
  }

  const bool* ObjLogState = State->get<LogMap>(SI.Obj);

  if (ObjLogState && *ObjLogState) {
    DBG("obj logged")
    // correct, do nothing
  } else {
    DBG("obj not logged")
    SI.reportNotLogBeforeWriteBug();
  }
}

void writeField(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  if (!SI.inTx) {
    DBG("not in tx")
    SI.reportAccessOutTxBug();
    return;
  }

  const bool* ObjLogState = State->get<LogMap>(SI.Obj);
  const bool* FieldLogState = State->get<LogMap>(SI.Field);

  if (ObjLogState && *ObjLogState) {
    DBG("obj logged")
    // correct, do nothing
  } else if (FieldLogState && *FieldLogState) {
    DBG("field logged")
    // correct, do nothing
  } else {
    DBG("not logged")
    SI.reportNotLogBeforeWriteBug();
  }
}

void logObj(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  if (!SI.inTx) {
    DBG("not in tx")
    SI.reportAccessOutTxBug();
    return;
  }

  const NamedDecl* Obj = SI.Obj;
  const bool* ObjLogState = State->get<LogMap>(Obj);
  if (ObjLogState && *ObjLogState) {
    DBG("obj logged")
    SI.reportDoubleLogBug();
  } else {
    DBG("obj not logged")
    State = State->set<LogMap>(Obj, true);
    SI.stateChanged = true;
  }
}

void logField(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  if (!SI.inTx) {
    DBG("not in tx")
    SI.reportAccessOutTxBug();
    return;
  }

  const NamedDecl* Obj = SI.Obj;
  const NamedDecl* Field = SI.Field;

  const bool* ObjLogState = State->get<LogMap>(Obj);
  const bool* FieldLogState = State->get<LogMap>(Field);

  if (ObjLogState && *ObjLogState) {
    DBG("obj logged")
    SI.reportDoubleLogBug();
  }else if(FieldLogState && *FieldLogState){
    DBG("field logged")
    SI.reportDoubleLogBug();
  }else{
    DBG("field nor obj not logged")
    State = State->set<LogMap>(Field, true);
    SI.stateChanged = true;
  }
}

} // namespace clang::ento::nvm::WriteSpace