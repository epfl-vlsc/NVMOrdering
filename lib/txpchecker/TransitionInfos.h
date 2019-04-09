#pragma once

#include "Common.h"
#include "DbgState.h"
#include "TxPBugReporter.h"

namespace clang::ento::nvm {

struct TransOutInfos {
  mutable bool stateChanged;
  mutable bool bugReported;

  TransOutInfos() : stateChanged(), bugReported(false) {}
};

struct TransInfos : public TransOutInfos {
  CheckerContext& C;
  ProgramStateRef& State;
  const NamedDecl* Current;
  const NamedDecl* Base;
  const TxPBugReporter& BR;
  SVal* Loc;
  const Stmt* S;

protected:
  TransInfos(CheckerContext& C_, ProgramStateRef& State_,
             const NamedDecl* Current_, const NamedDecl* Base_,
             const TxPBugReporter& BR_, SVal* Loc_, const Stmt* S_)
      : C(C_), State(State_), Current(Current_), Base(Base_), BR(BR_),
        Loc(Loc_), S(S_) {}
};

struct ReportInfos : public TransInfos {
private:
  void report(const BugPtr& bugPtr, const char* msg) const {
    DBG("report")
    if (ExplodedNode* EN = C.generateErrorNode()) {
      DBG("generate error node")
      BR.report(C, (const char*)Current, msg, Loc, EN, bugPtr);
    }
  }

  ReportInfos(CheckerContext& C_, ProgramStateRef& State_,
              const NamedDecl* Current_, const NamedDecl* Base_,
              const TxPBugReporter& BR_, SVal* Loc_, const Stmt* S_)
      : TransInfos(C_, State_, Current_, Base_, BR_, Loc_, S_) {}

public:
  static ReportInfos getRI(CheckerContext& C_, ProgramStateRef& State_,
                           const NamedDecl* Current_, const NamedDecl* Base_,
                           const TxPBugReporter& BR_, SVal* Loc_,
                           const Stmt* S_) {
    return ReportInfos(C_, State_, Current_, Base_, BR_, Loc_, S_);
  }

  void reportWriteOutTxBug() const {
    auto& bugPtr = BR.WriteOutTxBugType;
    report(bugPtr, "write outside tx");
  }

  void reportDoubleWriteBug() const {
    auto& bugPtr = BR.DoubleWriteBugType;
    report(bugPtr, "write twice");
  }

  void reportDoubleLogBugBug() const {
    auto& bugPtr = BR.DoubleLogBugType;
    report(bugPtr, "log twice");
  }

  void reportNotLogBeforeWriteBug() const {
    auto& bugPtr = BR.NotLogBeforeWriteBugType;
    report(bugPtr, "not logged before write");
  }
};

} // namespace clang::ento::nvm