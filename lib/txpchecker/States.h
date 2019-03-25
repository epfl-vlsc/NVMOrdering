#pragma once
#include "Common.h"

namespace clang::ento::nvm {

struct WriteState {
private:
  enum Kind { Init, Logged, Written } K;
  WriteState(Kind InK) : K(InK) {}

public:
  bool isInit() const { return K == Init; }
  bool isLogged() const { return K == Logged; }
  bool isWritten() const { return K == Written; }

  static WriteState getInit() { return WriteState(Init); }
  static WriteState getLogged() { return WriteState(Logged); }
  static WriteState getWritten() { return WriteState(Written); }

  bool operator==(const WriteState& X) const { return K == X.K; }
  void Profile(llvm::FoldingSetNodeID& ID) const { ID.AddInteger(K); }
};

} // namespace clang::ento::nvm

REGISTER_TRAIT_WITH_PROGRAMSTATE(TxCounter, unsigned)
REGISTER_MAP_WITH_PROGRAMSTATE(PMap, const clang::ento::MemRegion*,
                               clang::ento::nvm::WriteState)