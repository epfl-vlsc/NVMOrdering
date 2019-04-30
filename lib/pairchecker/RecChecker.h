#pragma once
#include "Common.h"
#include "DbgState.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.recchecker";

namespace clang::ento::nvm {

class RecChecker
    : public Checker<check::ASTDecl<TranslationUnitDecl>, check::BeginFunction,
                     check::PreCall, check::BranchCondition, check::Bind,
                     check::EndFunction> {

public:
  RecChecker() {}

  void checkASTDecl(const TranslationUnitDecl* CTUD, AnalysisManager& Mgr,
                    BugReporter& BR) const;

  void checkBeginFunction(CheckerContext& Ctx) const;

  void checkEndFunction(CheckerContext& C) const;

  void checkPreCall(const CallEvent& Call, CheckerContext& C) const;

  void checkBind(SVal Loc, SVal Val, const Stmt* S, CheckerContext& C) const;

  void checkBranchCondition(const Stmt* S, CheckerContext& C) const;

private:
  void addStateTransition(ProgramStateRef& State, CheckerContext& C,
                          bool stateChanged) const;

  void handleEnd(CheckerContext& C) const;
};

} // namespace clang::ento::nvm
