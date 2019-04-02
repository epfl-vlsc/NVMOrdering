#pragma once

#include "Common.h"
#include "DbgState.h"
#include "ReadBugReporter.h"

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
  const ReadBugReporter& BR;
  SVal* Loc;
  const Stmt* S;

  // must be set by the annotation infos!!!
  mutable char* D;

  const char* getD() const { return (const char*)D; }
  void setD(const char* D_) const { D = (char*)D_; }
  void setD(const ValueDecl* D_) const { D = (char*)D_; }
  void setD(const AnnotateAttr* D_) const { D = (char*)D_; }

  mutable bool mask;
  void setMask() const{
    mask = true;
  }
  bool useMask() const{
    return mask;
  }

  StringRef getVDName(){
    const ValueDecl* VD = getValueDecl(VarAddr);
    return VD->getName();
  }

protected:
  TransInfos(CheckerContext& C_, ProgramStateRef& State_, const char* VarAddr_,
             const ReadBugReporter& BR_, SVal* Loc_, const Stmt* S_)
      : C(C_), State(State_), VarAddr(VarAddr_), BR(BR_), Loc(Loc_), S(S_),
        D(nullptr), mask(false) {}
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
              const ReadBugReporter& BR_, SVal* Loc_, const Stmt* S_)
      : TransInfos(C_, State_, VarAddr_, BR_, Loc_, S_) {}

public:
  static ReportInfos getRI(CheckerContext& C_, ProgramStateRef& State_,
                           const char* VarAddr_, const ReadBugReporter& BR_,
                           SVal* Loc_, const Stmt* S_) {
    return ReportInfos(C_, State_, VarAddr_, BR_, Loc_, S_);
  }

  void reportCheckNotRead() const {
    auto& bugPtr = BR.CheckNotRead;
    report(bugPtr, "not read");
  }
  
};

} // namespace clang::ento::nvm
