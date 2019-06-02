#pragma once
#include "Common.h"

namespace clang::ento::nvm {




/*
class FunctionContext {
  const Stmt* Caller;
  const Stmt* Callee;

public:
  FunctionContext(const Stmt* Caller_, const Stmt* Callee_)
      : Caller(Caller_), Callee(Callee_) {}

  bool operator<(const Context& X) const {
    return (Caller < X.Caller && Callee < X.Callee);
  }

  bool operator==(const Context& X) const {
    return (Caller == X.Caller && Callee == X.Callee);
  }
};
*/

} // namespace clang::ento::nvm