#pragma once
#include "Common.h"
#include "DbgState.h"
#include "StateInfo.h"
#include "States.h"

namespace clang::ento::nvm::WriteSpace {

// todo handle writeobj

void initObj(StateInfo& SI) {
  /*
ProgramStateRef& State = SI.State;
const NamedDecl* Obj = SI.Obj;
*/
}

void writeField(StateInfo& SI) {
  /*
ProgramStateRef& State = SI.State;
const NamedDecl* Obj = SI.Obj;
const NamedDecl* Field = SI.Field;
*/
}

void logObj(StateInfo& SI) {
  /*
  ProgramStateRef& State = SI.State;
  const NamedDecl* Obj = SI.Obj;
  */
}

void logField(StateInfo& SI) {
  /*
ProgramStateRef& State = SI.State;
const NamedDecl* Field = SI.Field;
const WriteState* FieldWS = State->get<WriteMap>(Field);

DBG("logData field" << (void*)Field << " " << FieldWS)
*/
}

} // namespace clang::ento::nvm::WriteSpace