#pragma once
#include "Common.h"
#include "StateInOut.h"

namespace clang::ento::nvm {

template <typename BugReporter>
struct OrderState : public StateOut, public StateIn<BugReporter> {
  const char* VarAddr;
  mutable char* D;
  mutable ValueDecl* VD;
  mutable bool mask;

  OrderState(CheckerContext& C_, ProgramStateRef& State_,
             const BugReporter& BR_, SVal* Loc_, const Stmt* S_,
             const char* VarAddr_)
      : StateIn<BugReporter>(C_, State_, BR_, Loc_, S_), VarAddr(VarAddr_),
        D(nullptr), VD(nullptr), mask(false) {}

  const char* getD() const { return (const char*)D; }
  void setD(const char* D_) const { D = (char*)D_; }
  void setD(const ValueDecl* D_) const { D = (char*)D_; }
  void setD(const AnnotateAttr* D_) const { D = (char*)D_; }
  void setVD(const ValueDecl* VD_) const { VD = (ValueDecl*)VD_; }

  void setMask() const { mask = true; }
  bool useMask() const { return mask; }

  StringRef getVDName() {
    const ValueDecl* VD = getValueDecl(VarAddr);
    return VD->getName();
  }

  void report(const BugPtr& bugPtr, const char* msg) const {
    DBG("report")
    if (ExplodedNode* EN = this->C.generateErrorNode()) {
      DBG("generate error node")
      this->BR.report(this->C, VD, msg, this->Loc, EN, bugPtr);
    }
  }
};

} // namespace clang::ento::nvm