#pragma once
#include "Common.h"
#include "LatticeValue.h"

namespace clang::ento::nvm {

template <typename AbstractState>
bool transferWrite(const NamedDecl* ND, AbstractState& state) {
  state[ND] = LatticeValue::getWrite(state[ND]);
  return true;
}

template <typename AbstractState>
bool transferFlush(const NamedDecl* ND, AbstractState& state, bool isPfence) {
  auto& LV = state[ND];

  if (!LV.isWrite()) {
    return false;
  }

  if (isPfence) {
    state[ND] = LatticeValue::getPfence(LV);
  } else {
    state[ND] = LatticeValue::getFlush(LV);
  }
  return true;
}

} // namespace clang::ento::nvm