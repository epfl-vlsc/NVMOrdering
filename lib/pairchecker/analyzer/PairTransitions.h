#pragma once
#include "Common.h"
#include "PairTransfer.h"
#include "parser_util/ParseUtils.h"

namespace clang::ento::nvm {

template <typename Variables, typename Functions> class PairTransitions {
  using FunctionInfo = typename Functions::FunctionInfo;
  using LatVar = const NamedDecl*;
  using LatVal = LatticeValue;
  using AbstractState = std::map<LatVar, LatVal>;

  class PairTransitionInfo {
    bool useND;
    const NamedDecl* ND;

  public:
    PairTransitionInfo(bool useND_) : useND(useND_), ND(nullptr) {}
    PairTransitionInfo(const NamedDecl* ND_) : useND(true), ND(ND_) {
      assert(ND);
    }

    bool use() const { return useND; }

    const NamedDecl* getND() const { return ND; }
  };

  // useful structures
  Variables* vars;
  Functions* funcs;
  FunctionInfo* activeUnitInfo;
  AnalysisManager* Mgr;

  // parse-------------------------------------------------------
  PairTransitionInfo parseWrite(const BinaryOperator* BO) {
    const MemberExpr* ME = ParseUtils::getME(BO);
    if (!ME) {
      return PairTransitionInfo(false);
    }

    const ValueDecl* VD = ME->getMemberDecl();
    if (!activeUnitInfo->isUsedVar(VD)) {
      return PairTransitionInfo(false);
    }

    return PairTransitionInfo(VD);
  }

  PairTransitionInfo parseFlush(const CallExpr* CE) {
    const MemberExpr* ME = ParseUtils::getME(CE);
    if (!ME) {
      return PairTransitionInfo(false);
    }

    const ValueDecl* VD = ME->getMemberDecl();
    if (!activeUnitInfo->isUsedVar(VD)) {
      return PairTransitionInfo(false);
    }

    return PairTransitionInfo(VD);
  }

  // handle----------------------------------------------------
  bool handleWrite(const BinaryOperator* BO, AbstractState& state) {
    auto PTI = parseWrite(BO);

    return PTI.use() && transferWrite(PTI.getND(), state);
  }

  bool handleFence(const CallExpr* CE, AbstractState& state, bool isPfence) {
    llvm::report_fatal_error("not implemented");
    return false;
  }

  bool handleFlush(const CallExpr* CE, AbstractState& state, bool isPfence) {
    auto PTI = parseFlush(CE);

    return PTI.use() && transferFlush(PTI.getND(), state, isPfence);
  }

  bool handleCall(const CallExpr* CE, AbstractState& state) {
    const FunctionDecl* FD = CE->getDirectCallee();
    if (!FD)
      return false;

    if (funcs->isFlushFenceFunction(FD)) {
      return handleFlush(CE, state, true);
    } else if (funcs->isFlushOptFunction(FD)) {
      return handleFlush(CE, state, false);
    } else if (funcs->isVfenceFunction(FD)) {
      return handleFence(CE, state, false);
    } else if (funcs->isPfenceFunction(FD)) {
      return handleFence(CE, state, true);
    }

    return false;
  }

public:
  void initAll(Variables& vars_, Functions& funcs_, AnalysisManager* Mgr_) {
    vars = &vars_;
    funcs = &funcs_;
    Mgr = Mgr_;
  }

  void initUnit(FunctionInfo* activeUnitInfo_) {
    activeUnitInfo = activeUnitInfo_;
  }

  void initLatticeValues(AbstractState& state) {
    for (auto usedVar : activeUnitInfo->getUsedVars()) {
      auto& [isDcl, isScl] = vars->getDsclInfo(usedVar);
      auto lv = LatticeValue::getInit(isDcl, isScl);
      state[usedVar] = lv;
    }
  }

  bool handleStmt(const Stmt* S, AbstractState& state) {
    if (const BinaryOperator* BO = dyn_cast<BinaryOperator>(S)) {
      printStmt(S, *Mgr, "bo");
      return handleWrite(BO, state);
    } else if (const CallExpr* CE = dyn_cast<CallExpr>(S)) {
      printStmt(S, *Mgr, "ce");
      return handleCall(CE, state);
    }

    return false;
  }
};

} // namespace clang::ento::nvm