// transaction pmdk checker
#pragma once
#include "Common.h"
#include "States.h"
#include "TransactionInfos.h"
#include "Transitions.h"
#include "TxPBugReporter.h"
#include "DbgState.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.txpchecker";

namespace clang::ento::nvm {

class TxPChecker
    : public Checker<check::BeginFunction, check::EndFunction, check::Bind,
                     check::ASTDecl<FunctionDecl>, check::PostCall> {
public:
  TxPChecker() : BReporter(*this) {}

  void checkBeginFunction(CheckerContext& C) const;

  void checkEndFunction(CheckerContext& C) const;

  void checkBind(SVal Loc, SVal Val, const Stmt* S, CheckerContext& C) const;

  void checkPostCall(const CallEvent& Call, CheckerContext& C) const;

  void checkASTDecl(const FunctionDecl* D, AnalysisManager& Mgr,
                    BugReporter& BR) const;

private:
  void addStateTransition(ProgramStateRef& State, CheckerContext& C,
                          bool stateChanged) const;

  void handleTxBegin(const CallEvent& Call, CheckerContext& C) const;

  void handlePdirect(const CallEvent& Call, CheckerContext& C) const;

  void handleTxRangeDirect(const CallEvent& Call, CheckerContext& C) const;

  void handleTxRange(const CallEvent& Call, CheckerContext& C) const;

  void handleTxEnd(const CallEvent& Call, CheckerContext& C) const;

  bool inTx(ProgramStateRef& State) const;

  void handleEnd(CheckerContext& C) const;

/*
  void handlePalloc(const CallEvent& Call, CheckerContext& C) const;

  void handlePfree(const CallEvent& Call, CheckerContext& C) const;
*/

  template <typename SMap>
  void printStates(ProgramStateRef& State, CheckerContext& C) const;

  TxPBugReporter BReporter;
  mutable NVMTransactionInfo nvmTxInfo;
};

} // namespace clang::ento::nvm
