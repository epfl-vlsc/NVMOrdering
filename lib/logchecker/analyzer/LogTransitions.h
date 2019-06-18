#pragma once
#include "Common.h"
#include "LogBugReporter.h"
#include "LogLattice.h"
#include "dataflow_util/ProgramLocation.h"
#include "parser_util/ParseUtils.h"

namespace clang::ento::nvm {

template <typename Globals, typename StmtParser, typename BReporter>
class LogTransitions {
public:
  using LatVar = typename Globals::LatVar;
  using LatVal = LogLattice;
  using AbstractState = std::map<LatVar, LatVal>;
  using FunctionResults = std::map<ProgramLocation, AbstractState>;
  using DataflowResults = std::map<PlContext, FunctionResults>;
  using TransitionInfo = typename StmtParser::TransitionInfo;

private:
  // handle----------------------------------------------------
  bool handleWrite(const Stmt* S, const TransitionInfo& TI,
                   AbstractState& state) {
    LatVar var = TI.getVar();
    bugReporter.checkNotLoggedBug(var, S, state);
    return false;
  }

  bool handleLog(const Stmt* S, const TransitionInfo& TI,
                 AbstractState& state) {
    LatVar var = TI.getVar();
    bugReporter.checkDoubleLogBug(var, S, state);

    state[var] = LatVal::getLogged(state[var]);
    bugReporter.updateLastLocation(var, S);

    return true;
  }

  bool handleTxBegin(const Stmt* S, const TransitionInfo& TI,
                     AbstractState& state) {
    auto tx = globals.getTxFunction();
    state[tx] = LatVal::getBeginTx(state[tx]);

    return true;
  }

  bool handleTxEnd(const Stmt* S, const TransitionInfo& TI,
                   AbstractState& state) {
    auto tx = globals.getTxFunction();
    state[tx] = LatVal::getEndTx(state[tx]);

    return true;
  }

  bool handleStmt(const Stmt* S, const TransitionInfo& TI,
                  AbstractState& state) {
    switch (TI.getTransferFunction()) {
    case TransitionInfo::WriteFunc:
      return handleWrite(S, TI, state);
    case TransitionInfo::LogFunc:
      return handleLog(S, TI, state);
    case TransitionInfo::TxBeginFunc:
      return handleTxBegin(S, TI, state);
    case TransitionInfo::TxEndFunc:
      return handleTxEnd(S, TI, state);
    default:
      return false;
    }
  }

  Globals& globals;
  StmtParser& parser;
  BReporter& bugReporter;

public:
  LogTransitions(Globals& globals_, StmtParser& parser_,
                 BReporter& bugReporter_)
      : globals(globals_), parser(parser_), bugReporter(bugReporter_) {}

  void initLatticeValues(AbstractState& state) {
    // for tracking variables
    for (auto& var : globals.getUnitVariables()) {
      state[var] = LatVal::getInitVar();
    }

    auto tx = globals.getTxFunction();
    state[tx] = LatVal::getInitTx();
  }

  bool handleStmt(const Stmt* S, AbstractState& state) {
    // parse stmt to usable structure
    auto TI = parser.parseStmt(S);

    // handle stmt
    return handleStmt(S, TI, state);
  }
};

} // namespace clang::ento::nvm
