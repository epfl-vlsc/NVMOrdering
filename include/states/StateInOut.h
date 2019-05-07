#pragma once

#include "Common.h"
#include "DbgState.h"

namespace clang::ento::nvm {

struct StateOut {
  mutable bool stateChanged;
  mutable bool bugReported;

  StateOut() : stateChanged(false), bugReported(false) {}
};

template <typename BugReporter> struct StateIn {
  using BugPtr = std::unique_ptr<BugType>;

  CheckerContext& C;
  ProgramStateRef& State;
  const BugReporter& BR;
  const Stmt* S;

protected:
  StateIn(CheckerContext& C_, ProgramStateRef& State_, const BugReporter& BR_,
          const Stmt* S_)
      : C(C_), State(State_), BR(BR_), S(S_) {}
};

} // namespace clang::ento::nvm