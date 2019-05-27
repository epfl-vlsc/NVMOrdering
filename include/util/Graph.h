#pragma once
#include "Common.h"

namespace clang::ento::nvm {

bool endExploration(CheckerContext& C) {
  //if using this ensure that the callback completes afterwards
  ExplodedNode* ErrNode = C.generateErrorNode();
  return ErrNode == nullptr;
}

} // namespace clang::ento::nvm