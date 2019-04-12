#pragma once

#include "Common.h"
#include "DbgState.h"
#include "ReadBugReporter.h"
#include "states/OrderState.h"

namespace clang::ento::nvm {

struct StateInfo : public OrderState<ReadBugReporter> {
  StateInfo(CheckerContext& C_, ProgramStateRef& State_,
            const ReadBugReporter& BR_, SVal* Loc_, const Stmt* S_,
            const char* VarAddr_)
      : OrderState(C_, State_, BR_, Loc_, S_, VarAddr_) {}

  void reportCheckNotRead() const {
    auto& bugPtr = BR.CheckNotRead;
    report(bugPtr, "not read");
  }
};

} // namespace clang::ento::nvm
