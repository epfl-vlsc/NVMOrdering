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
      : StateIn(C_, State_, BR_, Loc_, S_), PI(PI_) {
  }

  void report(const BugPtr& bugPtr, const char* msg, const NamedDecl* ND) const {
    DBG("report")
    if (ExplodedNode* EN = this->C.generateErrorNode()) {
      DBG("generate error node")
      this->BR.report(this->C, ND, msg, this->Loc, EN, bugPtr);
    }
  }

  void reportDataAlreadyWritten() const {
    auto& bugPtr = BR.DataAlreadyWritten;
    report(bugPtr, "already written", this->PI->getDataND());
  }
  void reportDataNotWritten() const {
    auto& bugPtr = BR.DataNotWritten;
    report(bugPtr, "not written", this->PI->getDataND());
  }
  void reportCheckAlreadyWritten() const {
    auto& bugPtr = BR.CheckAlreadyWritten;
    report(bugPtr, "already written", this->PI->getCheckND());
  }
  void reportCheckNotWritten() const {
    auto& bugPtr = BR.CheckNotWritten;
    report(bugPtr, "not written", this->PI->getCheckND());
  }
  void reportDataNotPersisted() const {
    auto& bugPtr = BR.DataNotPersisted;
    report(bugPtr, "not persisted", this->PI->getDataND());
  }
  void reportDataAlreadyPersisted() const {
    auto& bugPtr = BR.DataAlreadyPersisted;
    report(bugPtr, "already persisted", this->PI->getDataND());
  }
  void reportModelBug(const std::string& msg) const {
    auto& bugPtr = BR.WrongModel;
    std::string errMsg = "model bug: " + msg + " state";
    report(bugPtr, errMsg.c_str(), this->PI->getDataND());
  }
};

} // namespace clang::ento::nvm
