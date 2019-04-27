// transaction pmdk checker
#pragma once
#include "Common.h"
#include "DbgState.h"
#include "parser/Parser.h"
#include "state_machine/Transitions.h"
#include "state_machine/TxpBugReporter.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.txpchecker";

namespace clang::ento::nvm {

class TxpChecker : public Checker<check::BeginFunction, check::EndFunction,
                                  check::Bind, check::ASTDecl<FunctionDecl>,
                                  check::PostCall, check::PreCall, eval::Call> {
public:
  TxpChecker() : BReporter(*this) {}

  void checkBeginFunction(CheckerContext& C) const;

  void checkEndFunction(CheckerContext& C) const;

  void checkBind(SVal Loc, SVal Val, const Stmt* S, CheckerContext& C) const;

  void checkPostCall(const CallEvent& Call, CheckerContext& C) const;

  void checkPreCall(const CallEvent& Call, CheckerContext& C) const;

  void checkASTDecl(const FunctionDecl* D, AnalysisManager& Mgr,
                    BugReporter& BR) const;

  bool evalCall(const CallExpr* CE, CheckerContext& C) const;

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

  void printStates(ProgramStateRef& State, CheckerContext& C) const;

  TxpBugReporter BReporter;
  mutable TxpFunctions txpFunctions;
};

} // namespace clang::ento::nvm
