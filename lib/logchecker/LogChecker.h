// transaction pmdk checker
#pragma once
#include "Common.h"
#include "identify/LogDecls.h"
#include "state_machine/LogBugReporter.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.logchecker";

namespace clang::ento::nvm {

class LogChecker
    : public Checker<check::ASTDecl<TranslationUnitDecl>, check::Bind,
                     check::PostCall, check::BeginFunction,
                     check::BranchCondition> {
public:
  LogChecker():BReporter(*this) {}

  void checkBind(SVal Loc, SVal Val, const Stmt* S, CheckerContext& C) const;

  void checkPostCall(const CallEvent& Call, CheckerContext& C) const;

  void checkASTDecl(const TranslationUnitDecl* CTUD, AnalysisManager& Mgr,
                    BugReporter& BR) const;

  void checkBeginFunction(CheckerContext& Ctx) const;

  void checkBranchCondition(const Stmt* S, CheckerContext& C) const;

private:
mutable LogDecls logDecls;
const LogBugReporter BReporter;
};

} // namespace clang::ento::nvm
