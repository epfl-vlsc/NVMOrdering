#pragma once

#include "Common.h"
#include "DbgState.h"
#include "WriteBugReporter.h"

namespace clang::ento::nvm {

struct TransOutInfos {
  mutable bool stateChanged;
  mutable bool bugReported;

  TransOutInfos() : stateChanged(), bugReported(false) {}
};

struct TransInfos : public TransOutInfos {
  CheckerContext& C;
  ProgramStateRef& State;
  const char* VarAddr;
  const WriteBugReporter& BR;
  SVal* Loc;
  const Stmt* S;

  // must be set by the annotation infos!!!
  mutable char* D;

  const char* getD() const { return (const char*)D; }
  void setD(const char* D_) const { D = (char*)D_; }
  void setD(const ValueDecl* D_) const { D = (char*)D_; }
  void setD(const AnnotateAttr* D_) const { D = (char*)D_; }

protected:
  TransInfos(CheckerContext& C_, ProgramStateRef& State_, const char* VarAddr_,
             const WriteBugReporter& BR_, SVal* Loc_, const Stmt* S_)
      : C(C_), State(State_), VarAddr(VarAddr_), BR(BR_), Loc(Loc_), S(S_),
        D(nullptr) {}
};

struct ReportInfos : public TransInfos {
private:
  void report(const BugPtr& bugPtr, const char* msg) const {
    DBG("report")
    if (ExplodedNode* EN = C.generateErrorNode()) {
      DBG("generate error node")
      BR.report(C, D, msg, Loc, EN, bugPtr);
    }
  }

  ReportInfos(CheckerContext& C_, ProgramStateRef& State_, const char* VarAddr_,
              const WriteBugReporter& BR_, SVal* Loc_, const Stmt* S_)
      : TransInfos(C_, State_, VarAddr_, BR_, Loc_, S_) {}

public:
  static ReportInfos getRI(CheckerContext& C_, ProgramStateRef& State_,
                           const char* VarAddr_, const WriteBugReporter& BR_,
                           SVal* Loc_, const Stmt* S_) {
    return ReportInfos(C_, State_, VarAddr_, BR_, Loc_, S_);
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
