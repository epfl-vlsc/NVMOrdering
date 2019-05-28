#pragma once
#include "Common.h"
#include "parser/Parser.h"
#include "state_machine/StateMachine.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.ptrchecker";

namespace clang::ento::nvm {

class PtrChecker
    : public Checker<check::ASTDecl<TranslationUnitDecl>, check::BeginFunction,
                     check::PreCall, check::BranchCondition, check::Bind,
                     check::EndFunction, eval::Call> {

public:
  PtrChecker() : BReporter(*this), ptrVars("persistent_ptr") {}

  void checkBeginFunction(CheckerContext& Ctx) const;

  void checkEndFunction(const ReturnStmt* RS, CheckerContext& C) const;

  void checkPreCall(const CallEvent& Call, CheckerContext& C) const;

  void checkBind(SVal Loc, SVal Val, const Stmt* S, CheckerContext& C) const;

  void checkASTDecl(const TranslationUnitDecl* CTUD, AnalysisManager& Mgr,
                    BugReporter& BR) const;

  void checkBranchCondition(const Stmt* S, CheckerContext& C) const;

  bool evalCall(const CallExpr* CE, CheckerContext& C) const;

private:
  void handleFlushFenceFnc(const CallEvent& Call, CheckerContext& C) const;

  const PtrBugReporter BReporter;
  mutable PtrFncs ptrFncs;
  mutable PtrVars ptrVars;
};

} // namespace clang::ento::nvm
