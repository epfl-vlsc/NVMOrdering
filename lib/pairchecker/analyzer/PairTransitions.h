#pragma once
#include "Common.h"
#include "PairBugReporter.h"
#include "dataflow_util/AbstractProgram.h"
#include "dataflow_util/TransitionChange.h"
#include "parser_util/ParseUtils.h"

namespace clang::ento::nvm {

template <typename Variables, typename Functions> class PairTransitions {
  using FunctionInfo = typename Functions::FunctionInfo;
  using LatVar = const NamedDecl*;
  using LatVal = PairLattice;
  using AbstractState = std::map<LatVar, LatVal>;
  using PBugReporter = PairBugReporter<AbstractState, Variables, Functions>;

  class PairTransitionInfo {
  public:
    enum TransferFunction {
      WriteFunc,
      FlushFunc,
      FlushFenceFunc,
      VfenceFunc,
      PfenceFunc,
      IpaFunc,
      NoFunc
    };

    static const constexpr char* tfStr[7] = {
        "Write", "Flush", "FlushFence", "Vfence", "Pfence", "Ipa", "No"};

  private:
    TransferFunction transferFunc;
    const NamedDecl* ND;

  public:
    PairTransitionInfo() : transferFunc(NoFunc), ND(nullptr) {}
    PairTransitionInfo(TransferFunction transferFunc_, const NamedDecl* ND_)
        : transferFunc(transferFunc_), ND(ND_) {
      assert(ND);
    }
    PairTransitionInfo(TransferFunction transferFunc_)
        : transferFunc(transferFunc_), ND(nullptr) {}

    void dump() const {
      if (transferFunc == NoFunc) {
        llvm::errs() << "Skip stmt\n";
      } else {
        llvm::errs() << tfStr[(int)transferFunc] << " function";
        printND(ND, "");
      }
    }

    static PairTransitionInfo getIpa() { return PairTransitionInfo(IpaFunc); }

    bool isStmtUsed() const { return transferFunc != NoFunc; }

    const NamedDecl* getND() const { return ND; }

    TransferFunction getTransferFunction() const { return transferFunc; }

    bool isPfence() const {
      return transferFunc == FlushFenceFunc || transferFunc == PfenceFunc;
    }

    static TransferFunction getWriteFunction() { return WriteFunc; }

    static TransferFunction getFlushFunction(bool isPfence) {
      return (isPfence) ? FlushFenceFunc : FlushFunc;
    }

    static TransferFunction getFenceFunction(bool isPfence) {
      return (isPfence) ? PfenceFunc : VfenceFunc;
    }
  };

  // useful structures
  Variables* vars;
  Functions* funcs;
  FunctionInfo* activeUnitInfo;
  AnalysisManager* Mgr;

  // for bug finding
  PBugReporter bugReporter;

  // for creating abstract graph
  bool useGlobal;

  void reportBugs() {}

  // parse-------------------------------------------------------
  bool isUsedVar(const NamedDecl* ND) {
    if (useGlobal) {
      return vars->isUsedVar(ND);
    } else {
      assert(activeUnitInfo && "active unit must be set");
      return activeUnitInfo->isUsedVar(ND);
    }
  }

  PairTransitionInfo parseWrite(const BinaryOperator* BO) {
    const MemberExpr* ME = ParseUtils::getME(BO);
    if (!ME) {
      return PairTransitionInfo();
    }

    const ValueDecl* VD = ME->getMemberDecl();
    if (!isUsedVar(VD)) {
      return PairTransitionInfo();
    }

    auto transferFunction = PairTransitionInfo::getWriteFunction();
    return PairTransitionInfo(transferFunction, VD);
  }

  PairTransitionInfo parseFlush(const CallExpr* CE, bool isPfence) {
    const MemberExpr* ME = ParseUtils::getME(CE);
    if (!ME) {
      return PairTransitionInfo();
    }

    const ValueDecl* VD = ME->getMemberDecl();
    if (!isUsedVar(VD)) {
      return PairTransitionInfo();
    }

    auto transferFunction = PairTransitionInfo::getFlushFunction(isPfence);
    return PairTransitionInfo(transferFunction, VD);
  }

  PairTransitionInfo parseFence(const CallExpr* CE, bool isPfence) {
    auto transferFunction = PairTransitionInfo::getFenceFunction(isPfence);
    return PairTransitionInfo(transferFunction);
  }

  PairTransitionInfo parseCall(const CallExpr* CE) {
    const FunctionDecl* FD = CE->getDirectCallee();
    if (!FD)
      return PairTransitionInfo();

    if (funcs->isFlushFenceFunction(FD)) {
      return parseFlush(CE, true);
    } else if (funcs->isFlushOptFunction(FD)) {
      return parseFlush(CE, false);
    } else if (funcs->isPfenceFunction(FD)) {
      return parseFence(CE, true);
    } else if (funcs->isVfenceFunction(FD)) {
      return parseFence(CE, false);
    } else if (useGlobal && !funcs->isSkipFunction(FD)) {
      return PairTransitionInfo::getIpa();
    }

    return PairTransitionInfo();
  }

  // handle----------------------------------------------------
  TransitionChange handleWrite(const AbstractStmt* absStmt,
                               const PairTransitionInfo& PTI,
                               AbstractState& state) {
    const NamedDecl* ND = PTI.getND();
    state[ND] = LatVal::getWrite(state[ND]);
    return bugReporter.updateLastLocation(ND, absStmt, state);
  }

  TransitionChange handleFence(const AbstractStmt* absStmt,
                               const PairTransitionInfo& PTI,
                               AbstractState& state) {
    bool isPfence = PTI.isPfence();
    bool stateChanged = false;

    for (auto& [Var, LV] : state) {
      if (isPfence && LV.hasDcl() && LV.isFlush()) {
        state[Var] = LatVal::getPfence(LV);
        bugReporter.updateLastLocation(Var, absStmt);
        stateChanged = true;
      }

      if (LV.hasScl() && LV.isWrite()) {
        state[Var] = LatVal::getVfence(LV);
        bugReporter.updateLastLocation(Var, absStmt);
        stateChanged = true;
      }
    }

    return getStateChange(stateChanged);
  }

  TransitionChange handleFlush(const AbstractStmt* absStmt,
                               const PairTransitionInfo& PTI,
                               AbstractState& state) {
    const NamedDecl* Var = PTI.getND();
    bool isPfence = PTI.isPfence();
    auto& LV = state[Var];
    bool stateChanged = false;

    if (isPfence && LV.hasDcl() && LV.isWriteFlushDcl()) {
      state[Var] = LatVal::getPfence(LV);
      bugReporter.updateLastLocation(Var, absStmt);
      stateChanged = true;
    } else if (LV.hasDcl() && LV.isWriteDcl()) {
      state[Var] = LatVal::getFlush(LV);
      bugReporter.updateLastLocation(Var, absStmt);
      stateChanged = true;
    }

    if (isPfence) {
      state[Var] = LatVal::getVfence(LV);
      bugReporter.updateLastLocation(Var, absStmt);
      stateChanged = true;
    }

    return getStateChange(stateChanged);
  }

  TransitionChange handleStmt(const AbstractStmt* absStmt,
                              const PairTransitionInfo& PTI,
                              AbstractState& state) {
    switch (PTI.getTransferFunction()) {
    case PairTransitionInfo::WriteFunc:
      return handleWrite(absStmt, PTI, state);
    case PairTransitionInfo::FlushFunc:
      return handleFlush(absStmt, PTI, state);
    case PairTransitionInfo::FlushFenceFunc:
      return handleFlush(absStmt, PTI, state);
    case PairTransitionInfo::VfenceFunc:
      return handleFence(absStmt, PTI, state);
    case PairTransitionInfo::PfenceFunc:
      return handleFence(absStmt, PTI, state);
    default:
      return TransitionChange::NoChange;
    }
  }

public:
  void initAll(Variables& vars_, Functions& funcs_, AnalysisManager* Mgr_,
               BugReporter* BR_, const CheckerBase* CB_) {
    vars = &vars_;
    funcs = &funcs_;
    Mgr = Mgr_;

    useGlobal = false;

    bugReporter.initAll(CB_, BR_, Mgr_, vars, funcs);
  }

  void useGlobalTransitions() { useGlobal = true; }

  void initUnit(FunctionInfo* activeUnitInfo_) {
    activeUnitInfo = activeUnitInfo_;
    useGlobal = false;

    bugReporter.initUnit(activeUnitInfo_);
  }

  void initLatticeValues(AbstractState& state) {
    for (auto& usedVar : activeUnitInfo->getUsedVars()) {
      auto& [isDcl, isScl] = vars->getDsclInfo(usedVar);
      auto lv = LatVal::getInit(isDcl, isScl);
      state[usedVar] = lv;
    }
  }

  PairTransitionInfo parseStmt(const Stmt* S) {
    if (const BinaryOperator* BO = dyn_cast<BinaryOperator>(S)) {
      return parseWrite(BO);
    } else if (const CallExpr* CE = dyn_cast<CallExpr>(S)) {
      return parseCall(CE);
    }

    return PairTransitionInfo();
  }

  TransitionChange handleStmt(const AbstractStmt* absStmt,
                              AbstractState& state) {
    // parse stmt to usable structure
    const Stmt* S = absStmt->getStmt();

    auto PTI = parseStmt(S);

    // handle stmt
    return handleStmt(absStmt, PTI, state);
  }

  // access structures
  auto getAnalysisFunctions() { return funcs->getAnalysisFunctions(); }

  AnalysisManager* getMgr() { return Mgr; }
};

} // namespace clang::ento::nvm
