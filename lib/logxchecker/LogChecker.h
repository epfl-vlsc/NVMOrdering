// transaction pmdk checker
#pragma once
#include "Common.h"
#include "parser/Parser.h"
#include "state_machine/StateMachine.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.logchecker";

namespace clang::ento::nvm {

class LogChecker
    : public Checker<check::ASTDecl<FunctionDecl>, check::ASTDecl<FieldDecl>,
                     check::Bind, check::PostCall, check::BeginFunction,
                     check::BranchCondition, eval::Call> {
public:
  LogChecker() : BReporter(*this) {}

  void checkASTDecl(const FunctionDecl* FD, AnalysisManager& Mgr,
                    BugReporter& BR) const;

  void checkASTDecl(const FieldDecl* FD, AnalysisManager& Mgr,
                    BugReporter& BR) const;

  void checkBind(SVal Loc, SVal Val, const Stmt* S, CheckerContext& C) const;

  void checkPostCall(const CallEvent& Call, CheckerContext& C) const;

  void checkBeginFunction(CheckerContext& Ctx) const;

  void checkBranchCondition(const Stmt* S, CheckerContext& C) const;

  bool evalCall(const CallExpr* CE, CheckerContext& C) const;

private:
  void handleLog(const CallEvent& Call, CheckerContext& C) const;

  void handleTxBeg(const CallEvent& Call, CheckerContext& C) const;

  void handleTxEnd(const CallEvent& Call, CheckerContext& C) const;

  void handleEnd(CheckerContext& C) const;

  void handleTx(ProgramStateRef& State, CheckerContext& C) const;

  const LogBugReporter BReporter;
  mutable LogFncs logFncs;
  mutable LogVars logVars;
};

} // namespace clang::ento::nvm
