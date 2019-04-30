#pragma once
#include "Common.h"

namespace clang::ento::nvm {

struct LogState {
private:
  enum Kind { Init, Logged, Written } K;
  LogState(Kind InK) : K(InK) {}

public:
  bool isLogged() const { return K == Logged; }
  bool isWritten() const { return K == Written; }
  bool isInit() const { return K == Init; }

  static LogState getLogged() { return LogState(Logged); }
  static LogState getWritten() { return LogState(Written); }
  static LogState getInit() { return LogState(Init); }

  bool operator==(const LogState& X) const { return K == X.K; }
  void Profile(llvm::FoldingSetNodeID& ID) const { ID.AddInteger(K); }
};

} // namespace clang::ento::nvm

REGISTER_MAP_WITH_PROGRAMSTATE(LogMap, const clang::FunctionDecl*,
                               clang::ento::nvm::LogState)