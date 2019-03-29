#pragma once

#include "Common.h"
#include "WriteBugReporter.h"

namespace clang::ento::nvm {

struct TransInfos {
  CheckerContext& C;
  ProgramStateRef& State;
  char* D;
  ExplodedNode*& EN;
  const WriteBugReporter& BR;

  ProgramStateRef getState() const { return C.getState(); }

protected:
  TransInfos(CheckerContext& C_, ProgramStateRef& State_, char* D_,
             ExplodedNode*& EN_, const WriteBugReporter& BR_)
      : C(C_), State(State_), D(D_), EN(EN_), BR(BR_) {}
};

struct WriteTransInfos : public TransInfos {
  SVal Loc;
  const Stmt* S;

  static WriteTransInfos getWTI(CheckerContext& C_, ProgramStateRef& State_,
                                char* D_, ExplodedNode*& EN_,
                                const WriteBugReporter& BR_, SVal Loc_,
                                const Stmt* S_) {
    return WriteTransInfos(C_, State_, D_, EN_, BR_, Loc_, S_);
  }

private:
  WriteTransInfos(CheckerContext& C_, ProgramStateRef& State_, char* D_,
                  ExplodedNode*& EN_, const WriteBugReporter& BR_, SVal Loc_,
                  const Stmt* S_)
      : TransInfos(C_, State_, D_, EN_, BR_), Loc(Loc_), S(S_) {}
};

} // namespace clang::ento::nvm
