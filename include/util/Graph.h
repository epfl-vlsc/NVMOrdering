#pragma once
#include "Common.h"

namespace clang::ento::nvm {

void endExploration(CheckerContext& C) {
  // if using this ensure that the callback completes afterwards
  C.generateErrorNode();
}

void addNewState(ProgramStateRef& State, ProgramStateRef& NewState,
                 CheckerContext& C) {
  if (NewState != State) {
    C.addTransition(NewState);
  }
}

} // namespace clang::ento::nvm