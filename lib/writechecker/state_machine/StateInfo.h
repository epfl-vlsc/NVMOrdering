#pragma once

#include "Common.h"
#include "DbgState.h"
#include "identify/StateInOut.h"
#include "WriteBugReporter.h"

namespace clang::ento::nvm {

struct StateInfo : public StateOut, public StateIn<WriteBugReporter> {
  const char* VarAddr;
  mutable char* D;
  mutable bool mask;

  StateInfo(CheckerContext& C_, ProgramStateRef& State_,
            const WriteBugReporter& BR_, SVal* Loc_, const Stmt* S_,
            const char* VarAddr_)
      : StateIn(C_, State_, BR_, Loc_, S_), VarAddr(VarAddr_), D(nullptr),
        mask(false) {}

  const char* getD() const { return (const char*)D; }
  void setD(const char* D_) const { D = (char*)D_; }
  void setD(const ValueDecl* D_) const { D = (char*)D_; }
  void setD(const AnnotateAttr* D_) const { D = (char*)D_; }

  void setMask() const { mask = true; }
  bool useMask() const { return mask; }

  StringRef getVDName() {
    const ValueDecl* VD = getValueDecl(VarAddr);
    return VD->getName();
  }

  void report(const BugPtr& bugPtr, const char* msg) const {
    DBG("report")
    if (ExplodedNode* EN = C.generateErrorNode()) {
      DBG("generate error node")
      BR.report(C, D, msg, Loc, EN, bugPtr);
    }
  } 

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
