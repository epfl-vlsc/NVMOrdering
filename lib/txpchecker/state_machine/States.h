#pragma once
#include "Common.h"

namespace clang::ento::nvm {

struct WriteState {
private:
  enum Kind { Init, Logged, Written } K;
  static constexpr const char* Str[] = {"Init", "Logged", "Written"};
  static constexpr const char* Exp[] = {"data is init", "data is logged",
                                        "data is written"};

  WriteState(Kind InK) : K(InK) {}

public:
  bool isInit() const { return K == Init; }
  bool isLogged() const { return K == Logged; }
  bool isWritten() const { return K == Written; }

  static WriteState getInit() { return WriteState(Init); }
  static WriteState getLogged() { return WriteState(Logged); }
  static WriteState getWritten() { return WriteState(Written); }

  int getStateKind() const { return static_cast<int>(K); }

  const char* getStateName() const { return Str[K]; }
  const char* getExplanation() const { return Exp[K]; }

  bool operator==(const WriteState& X) const { return K == X.K; }
  void Profile(llvm::FoldingSetNodeID& ID) const { ID.AddInteger(K); }
};

} // namespace clang::ento::nvm

REGISTER_TRAIT_WITH_PROGRAMSTATE(TxCounter, unsigned)
REGISTER_MAP_WITH_PROGRAMSTATE(WriteMap, const clang::NamedDecl*,
                               clang::ento::nvm::WriteState)
REGISTER_MAP_WITH_PROGRAMSTATE(VarMap, const clang::NamedDecl*,
                               const clang::NamedDecl*)