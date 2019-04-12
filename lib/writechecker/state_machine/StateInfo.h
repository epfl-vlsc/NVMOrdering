#pragma once

#include "Common.h"
#include "DbgState.h"
#include "WriteBugReporter.h"
#include "states/OrderState.h"

namespace clang::ento::nvm {

struct StateInfo : public OrderState<WriteBugReporter> {
  StateInfo(CheckerContext& C_, ProgramStateRef& State_,
            const WriteBugReporter& BR_, SVal* Loc_, const Stmt* S_,
            const char* VarAddr_)
      : OrderState(C_, State_, BR_, Loc_, S_, VarAddr_) {}

  void reportDataAlreadyWritten() const {
    auto& bugPtr = BR.DataAlreadyWritten;
    report(bugPtr, "already written");
  }
  void reportDataNotWritten() const {
    auto& bugPtr = BR.DataNotWritten;
    report(bugPtr, "not written");
  }
  void reportCheckAlreadyWritten() const {
    auto& bugPtr = BR.CheckAlreadyWritten;
    report(bugPtr, "already written");
  }
  void reportCheckNotWritten() const {
    auto& bugPtr = BR.CheckNotWritten;
    report(bugPtr, "not written");
  }
  void reportDataAlreadyFlushed() const {
    auto& bugPtr = BR.DataAlreadyFlushed;
    report(bugPtr, "already flushed");
  }
  void reportDataNotFlushed() const {
    auto& bugPtr = BR.DataNotFlushed;
    report(bugPtr, "not flushed");
  }
  void reportDataNotPersisted() const {
    auto& bugPtr = BR.DataNotPersisted;
    report(bugPtr, "not persisted");
  }
  void reportDataNotFenced() const {
    auto& bugPtr = BR.DataNotFenced;
    report(bugPtr, "not fenced");
  }
  void reportModelBug(const std::string& msg) const {
    auto& bugPtr = BR.WrongModel;
    std::string errMsg = "model bug: " + msg + " state";
    report(bugPtr, errMsg.c_str());
  }
};

} // namespace clang::ento::nvm
