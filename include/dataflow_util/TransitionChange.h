#pragma once
#include "Common.h"
#include "parser_util/ParseUtils.h"

namespace clang::ento::nvm {

enum class TransitionChange { StateChange, BugChange, NoChange };

} // namespace clang::ento::nvm
