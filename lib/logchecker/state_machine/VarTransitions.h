#pragma once
#include "Common.h"
#include "DbgState.h"
#include "LogState.h"

namespace clang::ento::nvm::VarSpace {

void writeTarget(CheckerContext& C, const NamedDecl* Alias, const NamedDecl* Target) {
  ProgramStateRef State = C.getState();
  const NamedDecl* const* TransTarget = State->get<VarMap>(Alias);

  if (TransTarget) {
    State = State->set<VarMap>(Alias, *TransTarget);
  } else {
    State = State->set<VarMap>(Alias, Target);
  }

  C.addTransition(State);
}

const NamedDecl* getTarget(CheckerContext& C, const NamedDecl* Alias) {
  ProgramStateRef State = C.getState();
  const NamedDecl* const* Target = State->get<VarMap>(Alias);

  if (Target) {
    return *Target;
  } else {
    return nullptr;
  }
}

} // namespace clang::ento::nvm::VarSpace