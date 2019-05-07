#pragma once
#include "Common.h"
#include "DbgState.h"
#include "parser/Parser.h"
#include "state_machine/StateMachine.h"

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
  struct HandleInfo {
    ProgramStateRef& State;
    CheckerContext& C;
    const Stmt* S;
    bool& stateChanged;
    bool& isCheck;
  };

  void addStateTransition(ProgramStateRef& State, const Stmt* S,
                          CheckerContext& C, bool stateChanged) const;

  template <bool fence>
  void handleFlushFnc(const CallEvent& Call, CheckerContext& C) const;

  template <bool pfence>
  void handleFence(const CallEvent& Call, CheckerContext& C) const;

  template <bool fence>
  void handleFlush(const NamedDecl* ND, HandleInfo& HI) const;

  void handleWrite(const NamedDecl* ND, HandleInfo& HI) const;

  void handleEnd(CheckerContext& C) const;

  const MainBugReporter BReporter;
  mutable MainFncs mainFncs;
  mutable MainVars mainVars;
};

} // namespace clang::ento::nvm
