#pragma once

#include "Common.h"
#include "States.h"
#include "TxpBugReporter.h"
#include "states/StateInOut.h"

namespace clang::ento::nvm {

struct StateInfo : public StateOut, public StateIn<TxpBugReporter> {
  const VarInfo& VI;

  StateInfo(CheckerContext& C_, ProgramStateRef& State_,
            const TxpBugReporter& BR_, const Stmt* S_, const VarInfo& VI_)
      : StateIn(C_, State_, BR_, S_), VI(VI_) {}

  void report(const BugPtr& bugPtr, const char* msg) const {
    DBG("report")
    if (ExplodedNode* EN = this->C.generateErrorNode()) {
      DBG("generate error node")
      BugReportData BR{nullptr, this->State, this->C, EN, msg, bugPtr};
      this->BR.report(BR);
    }
  }

  void reportAccessOutsideTxBug() const {
    auto& bugPtr = BR.AccessOutTxBugType;
    report(bugPtr, "access outside tx:");
  }

  void reportDoubleLogBug() const {
    auto& bugPtr = BR.DoubleLogBugType;
    report(bugPtr, "log twice:");
  }

  void reportNotLogBeforeWriteBug() const {
    auto& bugPtr = BR.NotLogBeforeWriteBugType;
    report(bugPtr, "not logged before write:");
  }

  void reportNotTxPairBug() const {
    auto& bugPtr = BR.NotTxPairBugType;
    report(bugPtr, "close tx end unnecessarily:");
  }
};

} // namespace clang::ento::nvm
