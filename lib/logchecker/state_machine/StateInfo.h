#pragma once
#include "Common.h"
#include "states/StateInOut.h"

namespace clang::ento::nvm {

template <typename BugReporter>
struct StateInfo : public StateOut, public StateIn<BugReporter> {
  const NamedDecl* ND;

  StateInfo(CheckerContext& C_, ProgramStateRef& State_, const BugReporter& BR_,
            SVal* Loc_, const Stmt* S_, const NamedDecl* ND_)
      : StateIn<BugReporter>(C_, State_, BR_, Loc_, S_), ND(ND_) {}

  void report(const BugPtr& bugPtr, const char* msg) const {
    DBG("report")
    if (ExplodedNode* EN = this->C.generateErrorNode()) {
      DBG("generate error node")
      this->BR.report(this->C, ND, msg, this->Loc, EN, bugPtr);
    }
  }
};

} // namespace clang::ento::nvm