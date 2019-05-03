#pragma once

#include "Common.h"
#include "DbgState.h"
#include "MainBugReporter.h"
#include "states/StateInOut.h"
#include "../parser/PairInfo.h"
namespace clang::ento::nvm {

struct StateInfo : public StateOut, public StateIn<MainBugReporter> {
public:
  const PairInfo* PI;
  const NamedDecl* ND;

private:
  void report(const BugPtr& bugPtr, const char* msg,
              const NamedDecl* ND) const {
    DBG("report")
    if (ExplodedNode* EN = this->C.generateErrorNode()) {
      DBG("generate error node")
      this->BR.report(this->C, ND, msg, this->Loc, EN, bugPtr);
    }
  }

  const NamedDecl* getReportND(bool isData) const {
    if (isData) {
      return this->PI->getDataND();
    } else {
      return this->PI->getCheckND();
    }
  }

public:
  StateInfo(CheckerContext& C_, ProgramStateRef& State_,
            const MainBugReporter& BR_, SVal* Loc_, const Stmt* S_,
            const PairInfo* PI_, const NamedDecl* ND_)
      : StateIn(C_, State_, BR_, Loc_, S_), PI(PI_), ND(ND_) {}

  bool isScl() const { return this->PI->isSameCl(); }

  bool isData() const { return this->PI->isData(this->ND); }

  void reportCommitBug(bool isData) const {
    auto& bugPtr = BR.CommitBug;
    const NamedDecl* RND = getReportND(isData);
    report(bugPtr, "already committed", RND);
  }
  void DoubleFlushBug() const {
    auto& bugPtr = BR.DoubleFlushBug;
    const NamedDecl* RND = getReportND(isData);
    report(bugPtr, "already committed", RND);
  }
  void FlushBug() const {
    auto& bugPtr = BR.FlushBug;
    const NamedDecl* RND = getReportND(isData);
    report(bugPtr, "flush", RND);
  }
};

} // namespace clang::ento::nvm
