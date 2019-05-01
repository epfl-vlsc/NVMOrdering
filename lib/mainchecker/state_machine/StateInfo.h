#pragma once

#include "Common.h"
#include "DbgState.h"
#include "MainBugReporter.h"
#include "states/StateInOut.h"

namespace clang::ento::nvm {

class PairInfo;

struct StateInfo : public StateOut, public StateIn<MainBugReporter> {
  const PairInfo* PI;

  StateInfo(CheckerContext& C_, ProgramStateRef& State_,
            const MainBugReporter& BR_, SVal* Loc_, const Stmt* S_,
            const PairInfo* PI_)
      : StateIn(C_, State_, BR_, Loc_, S_), PI(PI_) {}

  void report(const BugPtr& bugPtr, const char* msg,
              const NamedDecl* ND) const {
    DBG("report")
    if (ExplodedNode* EN = this->C.generateErrorNode()) {
      DBG("generate error node")
      this->BR.report(this->C, ND, msg, this->Loc, EN, bugPtr);
    }
  }

  void reportNotPossible(bool isData) const {
    auto& bugPtr = BR.NotPossible;
    const NamedDecl* ND =
        (isData) ? this->PI->getDataND() : this->PI->getCheckND();
    report(bugPtr, "already committed", ND);
  }
  void reportDataAlreadyCommitted() const {
    auto& bugPtr = BR.DataAlreadyCommitted;
    report(bugPtr, "already committed", this->PI->getDataND());
  }
  void reportDataNotCommitted() const {
    auto& bugPtr = BR.DataNotCommitted;
    report(bugPtr, "not committed", this->PI->getDataND());
  }
  void reportDataNotWritten() const {
    auto& bugPtr = BR.DataNotWritten;
    report(bugPtr, "not written", this->PI->getDataND());
  }
  void reportCheckAlreadyCommitted() const {
    auto& bugPtr = BR.CheckAlreadyCommitted;
    report(bugPtr, "already committed", this->PI->getCheckND());
  }
  void reportCheckNotCommitted() const {
    auto& bugPtr = BR.CheckNotCommitted;
    report(bugPtr, "not committed", this->PI->getCheckND());
  }
  void reportCheckNotWritten() const {
    auto& bugPtr = BR.CheckNotWritten;
    report(bugPtr, "not written", this->PI->getCheckND());
  }
  void reportModelBug(const std::string& msg) const {
    auto& bugPtr = BR.WrongModel;
    std::string errMsg = "model bug: " + msg + " state";
    report(bugPtr, errMsg.c_str(), this->PI->getDataND());
  }
};

} // namespace clang::ento::nvm
