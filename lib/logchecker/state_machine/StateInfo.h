#pragma once
#include "Common.h"
#include "LogBugReporter.h"
#include "states/StateInOut.h"

namespace clang::ento::nvm {

struct StateInfo : public StateOut, public StateIn<LogBugReporter> {
  const NamedDecl* ND;

  StateInfo(CheckerContext& C_, ProgramStateRef& State_,
            const LogBugReporter& BR_, SVal* Loc_, const Stmt* S_,
            const NamedDecl* ND_)
      : StateIn<LogBugReporter>(C_, State_, BR_, Loc_, S_), ND(ND_) {}

  void report(const BugPtr& bugPtr, const char* msg) const {
    DBG("report")
    if (ExplodedNode* EN = this->C.generateErrorNode()) {
      DBG("generate error node")
      this->BR.report(this->C, ND, msg, this->Loc, EN, bugPtr);
    }
  }

  void reportDoubleLogBug() const {
    auto& bugPtr = BR.DoubleLogBugType;
    report(bugPtr, "log twice:");
  }

  void reportNotLogBeforeWriteBug() const {
    auto& bugPtr = BR.NotLogBeforeWriteBugType;
    report(bugPtr, "not logged before write:");
  }

  void addTransition(){
    C.addTransition(State);
  }
};

} // namespace clang::ento::nvm