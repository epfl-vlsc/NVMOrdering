#pragma once
#include "Common.h"
#include "PairBugReporter.h"
#include "PairLattice.h"
#include "dataflow_util/ProgramLocation.h"
#include "parser_util/ParseUtils.h"

namespace clang::ento::nvm {

template <typename Globals, typename StmtParser, typename BReporter>
class PairTransitions {
public:
  using LatVar = typename Globals::LatVar;
  using LatVal = PairLattice;
  using AbstractState = std::map<LatVar, LatVal>;
  using FunctionResults = std::map<ProgramLocation, AbstractState>;
  using DataflowResults = std::map<PlContext, FunctionResults>;
  using TransitionInfo = typename StmtParser::TransitionInfo;

private:
  // handle----------------------------------------------------
  bool handleWrite(const Stmt* S, const TransitionInfo& TI,
                   AbstractState& state) {
    LatVar var = TI.getVar();
    state[var] = LatVal::getWrite(state[var]);
    bugReporter.updateLastLocation(var, S, state);
    return true;
  }

  bool handleFence(const Stmt* S, const TransitionInfo& TI,
                   AbstractState& state) {
    bool isPfence = TI.isPfence();
    bool stateChanged = false;

    for (auto& [Var, LV] : state) {
      if (isPfence && LV.hasDcl() && LV.isFlush()) {
        state[Var] = LatVal::getPfence(LV);
        bugReporter.updateLastLocation(Var, S);
        stateChanged = true;
      }

      if (LV.hasScl() && LV.isWrite()) {
        state[Var] = LatVal::getVfence(LV);
        bugReporter.updateLastLocation(Var, S);
        stateChanged = true;
      }
    }

    return stateChanged;
  }

  bool handleFlush(const Stmt* S, const TransitionInfo& TI,
                   AbstractState& state) {
    LatVar var = TI.getVar();
    bool isPfence = TI.isPfence();
    auto& LV = state[var];
    bool stateChanged = false;

    if (isPfence && LV.hasDcl() && LV.isWriteFlushDcl()) {
      state[var] = LatVal::getPfence(LV);
      bugReporter.updateLastLocation(var, S);
      stateChanged = true;
    } else if (LV.hasDcl() && LV.isWriteDcl()) {
      state[var] = LatVal::getFlush(LV);
      bugReporter.updateLastLocation(var, S);
      stateChanged = true;
    }

    if (isPfence) {
      state[var] = LatVal::getVfence(LV);
      bugReporter.updateLastLocation(var, S);
      stateChanged = true;
    }

    return stateChanged;
  }

  bool handleStmt(const Stmt* S, const TransitionInfo& TI,
                  AbstractState& state) {
    switch (TI.getTransferFunction()) {
    case TransitionInfo::WriteFunc:
      return handleWrite(S, TI, state);
    case TransitionInfo::FlushFunc:
      return handleFlush(S, TI, state);
    case TransitionInfo::FlushFenceFunc:
      return handleFlush(S, TI, state);
    case TransitionInfo::VfenceFunc:
      return handleFence(S, TI, state);
    case TransitionInfo::PfenceFunc:
      return handleFence(S, TI, state);
    default:
      return false;
    }
  }

  Globals& globals;
  StmtParser& parser;
  BReporter& bugReporter;

public:
  PairTransitions(Globals& globals_, StmtParser& parser_,
                  BReporter& bugReporter_)
      : globals(globals_), parser(parser_), bugReporter(bugReporter_) {}

  void initLatticeValues(AbstractState& state) {
    for (auto& usedVar : globals.getUnitVariables()) {
      auto& [isDcl, isScl] = globals.getDsclInfo(usedVar);
      auto lv = LatVal::getInit(isDcl, isScl);
      state[usedVar] = lv;
    }
  }

  bool handleStmt(const Stmt* S, AbstractState& state) {
    // parse stmt to usable structure
    auto TI = parser.parseStmt(S);

    // handle stmt
    return handleStmt(S, TI, state);
  }
};

} // namespace clang::ento::nvm
