#pragma once
#include "Common.h"
#include "FlushedBugReporter.h"
#include "FlushedLattice.h"
#include "dataflow_util/ProgramLocation.h"
#include "parser_util/ParseUtils.h"

namespace clang::ento::nvm {

template <typename Globals, typename StmtParser, typename BReporter>
class FlushedTransitions {
public:
  using LatVar = typename Globals::LatVar;
  using LatVal = FlushedLattice;
  using AbstractState = std::map<LatVar, LatVal>;
  using FunctionResults = std::map<ProgramLocation, AbstractState>;
  using DataflowResults = std::map<PlContext, FunctionResults>;
  using TransitionInfo = typename StmtParser::TransitionInfo;

private:
  // handle----------------------------------------------------
  bool handleWrite(const Stmt* S, const TransitionInfo& TI,
                   AbstractState& state) {
    if (TI.getAccessType() == TransitionInfo::FieldAccess) {
      LatVar fieldvar = TI.getTrackedVariable();
      LatVar trackedvar = TI.getTrackedVariable();
      bugReporter.checkBug(fieldvar, trackedvar, S, state);
    } else if (TI.getAccessType() == TransitionInfo::TrackedvariableAccess) {
      LatVar var = TI.getTrackedVariable();
      state[var] = LatVal::getWrite(state[var]);
      bugReporter.updateLastLocation(var, S);
    }

    return true;
  }

  bool handleFence(const Stmt* S, const TransitionInfo& TI,
                   AbstractState& state) {
    bool stateChanged = false;

    for (auto& [var, lv] : state) {
      if (lv.isFlush()) {
        state[var] = LatVal::getPfence(lv);
        bugReporter.updateLastLocation(var, S);
        stateChanged = true;
      }
    }

    return stateChanged;
  }

  bool handleFlush(const Stmt* S, const TransitionInfo& TI,
                   AbstractState& state) {
    if (TI.getAccessType() != TransitionInfo::TrackedVariableAccess) {
      return false;
    }
    LatVar var = TI.getTrackedVariable();
    bool isPfence = TI.isPfence();

    auto& lv = state[var];

    bool stateChanged = false;
    if (isPfence && lv.isWriteFlush()) {
      state[var] = LatVal::getPfence(lv);
      bugReporter.updateLastLocation(var, S);
      stateChanged = true;
    } else if (!isPfence && lv.isWriteFlush()) {
      state[var] = LatVal::getFlush(lv);
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
  FlushedTransitions(Globals& globals_, StmtParser& parser_,
                     BReporter& bugReporter_)
      : globals(globals_), parser(parser_), bugReporter(bugReporter_) {}

  void initLatticeValues(AbstractState& state) {
    for (auto& usedvar : globals.getUnitvariables()) {
      auto lv = LatVal::getInit();
      state[usedvar] = lv;
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
