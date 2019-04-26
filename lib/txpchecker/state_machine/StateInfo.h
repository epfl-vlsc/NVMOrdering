#pragma once

#include "Common.h"
#include "DbgState.h"
#include "TxpBugReporter.h"
#include "states/StateInOut.h"

namespace clang::ento::nvm {

struct StateInfo : public StateOut, public StateIn<TxpBugReporter> {
  const NamedDecl* Obj;
  const NamedDecl* Field;
  bool inTx;

  StateInfo(CheckerContext& C_, ProgramStateRef& State_,
            const TxpBugReporter& BR_, SVal* Loc_, const Stmt* S_,
            const NamedDecl* Obj_, const NamedDecl* Field_, bool inTx_)
      : StateIn(C_, State_, BR_, Loc_, S_), Obj(Obj_), Field(Field_),
        inTx(inTx_) {}

  void report(const BugPtr& bugPtr, const char* msg) const {
    DBG("report")
    if (ExplodedNode* EN = this->C.generateErrorNode()) {
      DBG("generate error node")

      this->BR.report(this->C, this->Obj, this->Field, msg, this->Loc, EN,
                      bugPtr);
    }
  }

  void reportWriteOutTxBug() const {
    auto& bugPtr = BR.WriteOutTxBugType;
    report(bugPtr, "write outside tx:");
  }

  void reportDoubleWriteBug() const {
    auto& bugPtr = BR.DoubleWriteBugType;
    report(bugPtr, "write twice:");
  }

  void reportDoubleLogBugBug() const {
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
