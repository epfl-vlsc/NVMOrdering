#pragma once
#include "Common.h"
#include "LatticeValue.h"
#include "parser_util/ParseUtils.h"

namespace clang::ento::nvm {

template <typename Variables, typename Functions> class PairTransitions {
  using FunctionInfo = typename Functions::FunctionInfo;
  using LatVar = const NamedDecl*;
  using LatVal = LatticeValue;
  using AbstractState = std::map<LatVar, LatVal>;

  // useful structures
  Variables* vars;
  Functions* funcs;
  FunctionInfo* activeUnitInfo;
  AnalysisManager* Mgr;

  bool handleWrite(const BinaryOperator* BO, AbstractState& state) {
    const MemberExpr* ME = ParseUtils::getME(BO);
    if (!ME) {
      return false;
    }

    const ValueDecl* VD = ME->getMemberDecl();
    if (!activeUnitInfo->isUsedVar(VD)) {
      return false;
    }

    state[VD] = LatticeValue::getWrite(state[VD]);
    return true;
  }

  bool handleFence(const CallExpr* CE, AbstractState& state, bool isPfence) {
    llvm::report_fatal_error("not implemented");
    return false;
  }

  bool handleFlush(const CallExpr* CE, AbstractState& state, bool isPfence) {
    const MemberExpr* ME = ParseUtils::getME(CE);
    if (!ME) {
      return false;
    }

    const ValueDecl* VD = ME->getMemberDecl();
    if (!activeUnitInfo->isUsedVar(VD)) {
      return false;
    }

    if (isPfence) {
      state[VD] = LatticeValue::getPfence(state[VD]);
    } else {
      state[VD] = LatticeValue::getFlush(state[VD]);
    }
    return true;
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