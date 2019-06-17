#pragma once
#include "Common.h"
#include "parser_util/ParseUtils.h"

namespace clang::ento::nvm {

enum class TransitionChange { StateChange, BugChange, NoChange };

TransitionChange getStateChange(bool stateChanged) {
  if (stateChanged) {
    return TransitionChange::StateChange;
  } else {
    return TransitionChange::NoChange;
  }
}

} // namespace clang::ento::nvm
