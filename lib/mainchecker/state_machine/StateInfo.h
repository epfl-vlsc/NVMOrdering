#pragma once

#include "Common.h"
#include "DbgState.h"
#include "MainBugReporter.h"
#include "states/StateInOut.h"

namespace clang::ento::nvm {

struct StateInfo : public StateOut, public StateIn<MainBugReporter> {
  const NamedDecl* dataND;
  const NamedDecl* checkND;

  StateInfo(CheckerContext& C_, ProgramStateRef& State_,
            const MainBugReporter& BR_, SVal* Loc_, const Stmt* S_,
            const NamedDecl* dataND_, const NamedDecl* checkND_)
      : StateIn(C_, State_, BR_, Loc_, S_), dataND(dataND_), checkND(checkND_) {
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
    report(bugPtr, "already written", this->dataND);
  }
  void reportDataNotWritten() const {
    auto& bugPtr = BR.DataNotWritten;
    report(bugPtr, "not written", this->dataND);
  }
  void reportCheckAlreadyWritten() const {
    auto& bugPtr = BR.CheckAlreadyWritten;
    report(bugPtr, "already written", this->checkND);
  }
  void reportCheckNotWritten() const {
    auto& bugPtr = BR.CheckNotWritten;
    report(bugPtr, "not written", this->checkND);
  }
  void reportDataNotPersisted() const {
    auto& bugPtr = BR.DataNotPersisted;
    report(bugPtr, "not persisted", this->dataND);
  }
  void reportDataAlreadyPersisted() const {
    auto& bugPtr = BR.DataAlreadyPersisted;
    report(bugPtr, "already persisted", this->dataND);
  }
  void reportModelBug(const std::string& msg) const {
    auto& bugPtr = BR.WrongModel;
    std::string errMsg = "model bug: " + msg + " state";
    report(bugPtr, errMsg.c_str(), this->dataND);
  }
};

} // namespace clang::ento::nvm
