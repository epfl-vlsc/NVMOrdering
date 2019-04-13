#pragma once
#include "WriteWalkers.h"
#include "Common.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.writechecker";

namespace clang::ento::nvm {

class WriteChecker
    : public Checker<check::ASTDecl<TranslationUnitDecl>, check::BeginFunction,
                     check::PreCall, check::BranchCondition, check::Bind,
                     check::EndFunction> {

public:
  WriteChecker() : BReporter(*this) {}

  void checkBeginFunction(CheckerContext& Ctx) const;

  void checkEndFunction(CheckerContext& C) const;

  void checkPreCall(const CallEvent& Call, CheckerContext& C) const;

  void checkBind(SVal Loc, SVal Val, const Stmt* S, CheckerContext& C) const;

  void checkASTDecl(const TranslationUnitDecl* CTUD, AnalysisManager& Mgr,
                    BugReporter& BR) const;

  void checkBranchCondition(const Stmt* S, CheckerContext& C) const;

private:
  void addStateTransition(ProgramStateRef& State, CheckerContext& C,
                          bool stateChanged) const;

  void handleFlush(const CallEvent& Call, CheckerContext& C) const;

  template <bool pfence>
  void handleFence(const CallEvent& Call, CheckerContext& C) const;

  template <typename SMap>
  void checkMapStates(ProgramStateRef& State, CheckerContext& C) const;

  template <typename SMap, bool pfence>
  void checkFenceStates(ProgramStateRef& State, CheckerContext& C,
                        bool& stateChanged) const;

  template <typename SMap>
  void printStates(ProgramStateRef& State, CheckerContext& C) const;

  void handleEnd(CheckerContext& C) const;

  const WriteBugReporter BReporter;
  mutable OrderFncs orderFncs;
  mutable OrderVarsBI orderVars;
};

} // namespace clang::ento::nvm
