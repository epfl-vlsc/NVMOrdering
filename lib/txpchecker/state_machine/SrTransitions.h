#pragma once
#include "Common.h"
#include "reporting/SourceLineStorage.h"

namespace clang::ento::nvm::SrSpace {

void addSRToSlStore(ProgramStateRef& State, const SourceRange& SR,
                    const FunctionDecl* FD) {
  State = State->set<SlStore>(SlSpace::SlRange::getSlRange(SR), FD);
}

bool isSRInSlStore(const ProgramStateRef& State, const SourceRange& SR) {
  auto slRange = SlSpace::SlRange::getSlRange(SR);
  return State->contains<SlStore>(slRange);
}

bool removeSRFromMap(ProgramStateRef& State, const FunctionDecl* FD) {
  bool stateChanged = false;
  for (auto& [SR, SRFD] : State->get<SlStore>()) {
    if (FD == SRFD) {
      State = State->remove<SlStore>(SR);
      stateChanged = true;
    }
  }
  return stateChanged;
}

} // namespace clang::ento::nvm::SrSpace