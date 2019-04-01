#pragma once
#include "Common.h"

namespace clang::ento::nvm {

struct WriteState {
private:
  enum Kind { Logged, Written } K;
  WriteState(Kind InK) : K(InK) {}

public:
  bool isLogged() const { return K == Logged; }
  bool isWritten() const { return K == Written; }

  static WriteState getLogged() { return WriteState(Logged); }
  static WriteState getWritten() { return WriteState(Written); }

  bool operator==(const WriteState& X) const { return K == X.K; }
  void Profile(llvm::FoldingSetNodeID& ID) const { ID.AddInteger(K); }
};

} // namespace clang::ento::nvm

REGISTER_MAP_WITH_PROGRAMSTATE(LogMap, const clang::ValueDecl*,
                               clang::ento::nvm::WriteState)

REGISTER_TRAIT_WITH_PROGRAMSTATE(Logged, bool)