#pragma once
#include "Common.h"
#include "DbgState.h"
#include "parser/Parser.h"
#include "state_machine/MainBugReporter.h"
#include "state_machine/WriteState.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.mainchecker";

namespace clang::ento::nvm {

class MainChecker
    : public Checker<check::ASTDecl<TranslationUnitDecl>, check::BeginFunction,
                     check::PreCall, check::BranchCondition, check::Bind,
                     check::EndFunction, eval::Call> {

public:
  MainChecker() : BReporter(*this) {}

  void checkBeginFunction(CheckerContext& Ctx) const;

  void checkEndFunction(CheckerContext& C) const;

  void checkPreCall(const CallEvent& Call, CheckerContext& C) const;

  void checkBind(SVal Loc, SVal Val, const Stmt* S, CheckerContext& C) const;

  void checkASTDecl(const TranslationUnitDecl* CTUD, AnalysisManager& Mgr,
                    BugReporter& BR) const;

  void checkBranchCondition(const Stmt* S, CheckerContext& C) const;

  bool evalCall(const CallExpr* CE, CheckerContext& C) const;

private:
  void addStateTransition(ProgramStateRef& State, CheckerContext& C,
                          bool stateChanged) const;

  void handleFenceFlush(const CallEvent& Call, CheckerContext& C) const;

  void handleEnd(CheckerContext& C) const;

  const MainBugReporter BReporter;
  mutable MainFncs mainFncs;
  mutable MainVars mainVars;
};

} // namespace clang::ento::nvm
