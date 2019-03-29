#pragma once

#include "Common.h"
#include "WriteBugReporter.h"

namespace clang::ento::nvm {

struct TransOutInfos {
  bool stateChanged;
  bool bugReported;

  TransOutInfos() : stateChanged(), bugReported(false) {}
};

struct TransInfos : public TransOutInfos {
  CheckerContext& C;
  ProgramStateRef& State;
  char* D;
  ExplodedNode*& EN;
  const WriteBugReporter& BR;

  ProgramStateRef getState() const { return C.getState(); }

  const WriteBugReporter& initReport() {
    bugReported = true;
    if (!EN) {
      EN = C.generateNonFatalErrorNode();
    }
    return BR;
  }

protected:
  TransInfos(CheckerContext& C_, ProgramStateRef& State_, char* D_,
             ExplodedNode*& EN_, const WriteBugReporter& BR_)
      : C(C_), State(State_), D(D_), EN(EN_), BR(BR_) {}
};

struct WriteTransInfos : public TransInfos {
  SVal Loc;
  const Stmt* S;

protected:
  WriteTransInfos(CheckerContext& C_, ProgramStateRef& State_, char* D_,
                  ExplodedNode*& EN_, const WriteBugReporter& BR_, SVal Loc_,
                  const Stmt* S_)
      : TransInfos(C_, State_, D_, EN_, BR_), Loc(Loc_), S(S_) {}
};

struct ReportInfos : public WriteTransInfos {
  static ReportInfos getRI(CheckerContext& C_, ProgramStateRef& State_,
                           char* D_, ExplodedNode*& EN_,
                           const WriteBugReporter& BR_, SVal Loc_,
                           const Stmt* S_) {
    return ReportInfos(C_, State_, D_, EN_, BR_, Loc_, S_);
  }

  void reportDataAlreadyWritten() const {
    auto& bugPtr = BR.DataAlreadyWritten;
    BR.report(C, D, "already written", Loc, EN, bugPtr);
  }

  void reportDataNotWritten() const {
    auto& bugPtr = BR.DataNotWritten;
    BR.report(C, D, "already written", Loc, EN, bugPtr);
  }
  void reportDataAlreadyFenced() const {
    auto& bugPtr = BR.DataAlreadyFenced;
    BR.report(C, D, "already written", Loc, EN, bugPtr);
  }
  void reportDataNotFenced() const {
    auto& bugPtr = BR.DataNotFenced;
    BR.report(C, D, "already written", Loc, EN, bugPtr);
  }
  void reportCheckAlreadyWritten() const {
    auto& bugPtr = BR.CheckAlreadyWritten;
    BR.report(C, D, "already written", Loc, EN, bugPtr);
  }
  void reportCheckNotWritten() const {
    auto& bugPtr = BR.CheckNotWritten;
    BR.report(C, D, "already written", Loc, EN, bugPtr);
  }
  void reportDataAlreadyFlushed() const {
    auto& bugPtr = BR.DataAlreadyFlushed;
    BR.report(C, D, "already written", Loc, EN, bugPtr);
  }
  void reportDataNotFlushed() const {
    auto& bugPtr = BR.DataNotFlushed;
    BR.report(C, D, "already written", Loc, EN, bugPtr);
  }
  void reportDataNotPersisted() const {
    auto& bugPtr = BR.DataNotPersisted;
    BR.report(C, D, "already written", Loc, EN, bugPtr);
  }

private:
  ReportInfos(CheckerContext& C_, ProgramStateRef& State_, char* D_,
              ExplodedNode*& EN_, const WriteBugReporter& BR_, SVal Loc_,
              const Stmt* S_)
      : WriteTransInfos(C_, State_, D_, EN_, BR_, Loc_, S_) {}
};

} // namespace clang::ento::nvm
