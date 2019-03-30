#pragma once

#include "Common.h"
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
  ExplodedNode*& EN;
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
             ExplodedNode*& EN_, const WriteBugReporter& BR_, SVal* Loc_,
             const Stmt* S_)
      : C(C_), State(State_), VarAddr(VarAddr_), EN(EN_), BR(BR_), Loc(Loc_),
        S(S_), D(nullptr) {}
};

struct ReportInfos : public TransInfos {
private:
  void initReport() const {
    bugReported = true;
    if (!EN) {
      EN = C.generateNonFatalErrorNode();
    }
  }

  ReportInfos(CheckerContext& C_, ProgramStateRef& State_, const char* VarAddr_,
              ExplodedNode*& EN_, const WriteBugReporter& BR_, SVal* Loc_,
              const Stmt* S_)
      : TransInfos(C_, State_, VarAddr_, EN_, BR_, Loc_, S_) {}

public:
  static ReportInfos getRI(CheckerContext& C_, ProgramStateRef& State_,
                           const char* VarAddr_, ExplodedNode*& EN_,
                           const WriteBugReporter& BR_, SVal* Loc_,
                           const Stmt* S_) {
    return ReportInfos(C_, State_, VarAddr_, EN_, BR_, Loc_, S_);
  }

  void reportDataAlreadyWritten() const {
    initReport();
    if (!EN)
      return;
    auto& bugPtr = BR.DataAlreadyWritten;
    BR.report(C, D, "already written", Loc, EN, bugPtr);
  }
  void reportDataNotWritten() const {
    initReport();
    if (!EN)
      return;
    auto& bugPtr = BR.DataNotWritten;
    BR.report(C, D, "not written", Loc, EN, bugPtr);
  }
  void reportCheckAlreadyWritten() const {
    initReport();
    if (!EN)
      return;
    auto& bugPtr = BR.CheckAlreadyWritten;
    BR.report(C, VarAddr, "already written", Loc, EN, bugPtr);
  }
  void reportCheckNotWritten() const {
    initReport();
    if (!EN)
      return;
    auto& bugPtr = BR.CheckNotWritten;
    BR.report(C, VarAddr, "not written", Loc, EN, bugPtr);
  }
  void reportDataAlreadyFlushed() const {
    initReport();
    if (!EN)
      return;
    auto& bugPtr = BR.DataAlreadyFlushed;
    BR.report(C, D, "already flushed", Loc, EN, bugPtr);
  }
  void reportDataNotFlushed() const {
    initReport();
    if (!EN)
      return;
    auto& bugPtr = BR.DataNotFlushed;
    BR.report(C, D, "not flushed", Loc, EN, bugPtr);
  }
  void reportDataNotPersisted() const {
    initReport();
    if (!EN)
      return;
    auto& bugPtr = BR.DataNotPersisted;
    BR.report(C, D, "not persisted", Loc, EN, bugPtr);
  }
  void reportDataNotFenced() const {
    initReport();
    if (!EN)
      return;
    auto& bugPtr = BR.DataNotFenced;
    BR.report(C, D, "not fenced", Loc, EN, bugPtr);
  }

  void reportModelBug(const std::string& msg) const {
    initReport();
    if (!EN)
      return;
    auto& bugPtr = BR.WrongModel;
    std::string errMsg = "model bug: " + msg + " state";
    BR.report(C, VarAddr, errMsg.c_str(), Loc, EN, bugPtr);
  }
};

} // namespace clang::ento::nvm
