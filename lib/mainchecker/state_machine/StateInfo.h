#pragma once

#include "../parser/PairInfo.h"
#include "Common.h"
#include "MainBugReporter.h"
#include "WriteState.h"
#include "states/StateInOut.h"
namespace clang::ento::nvm {

struct StateInfo : public StateOut, public StateIn<MainBugReporter> {
public:
  const PairInfo* PI;
  const NamedDecl* ND;

private:
  void report(const BugPtr& bugPtr, const char* msg,
              const NamedDecl* ND) const {
    DBG("report bug " << msg)
    if (ExplodedNode* EN = this->C.generateErrorNode()) {
      BugReportData BRData{ND, this->State, this->C, EN, msg, bugPtr};
      this->BR.report(BRData);
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
            const MainBugReporter& BR_, const Stmt* S_, const PairInfo* PI_,
            const NamedDecl* ND_)
      : StateIn(C_, State_, BR_, S_), PI(PI_), ND(ND_) {}

  bool isScl() const { return this->PI->isSameCl(); }

  bool isDcl() const { return !this->PI->isSameCl(); }

  bool isData() const { return this->PI->isData(this->ND); }

  bool isCheck() const { return this->PI->isCheck(this->ND); }

  void reportWriteBug(bool isData) const {
    auto& bugPtr = BR.WriteBug;
    const NamedDecl* RND = getReportND(isData);
    report(bugPtr, "write", RND);
  }
  void reportCommitBug(bool isData) const {
    auto& bugPtr = BR.CommitBug;
    const NamedDecl* RND = getReportND(isData);
    report(bugPtr, "commit", RND);
  }
  void reportDoubleFlushBug(bool isData) const {
    auto& bugPtr = BR.DoubleFlushBug;
    const NamedDecl* RND = getReportND(isData);
    report(bugPtr, "double flush", RND);
  }
  void reportFlushBug(bool isData) const {
    auto& bugPtr = BR.FlushBug;
    const NamedDecl* RND = getReportND(isData);
    report(bugPtr, "flush", RND);
  }

  void doTransition(WriteState::Kind K) const {
    ProgramStateRef& NewState = this->State;
    NewState = NewState->set<WriteMap>(PI, WriteState::getNewState(K));
    this->stateChanged = true;
  }
};

} // namespace clang::ento::nvm
