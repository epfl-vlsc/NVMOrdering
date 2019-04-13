// transaction pmdk checker
#pragma once
#include "Common.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.logchecker";

namespace clang::ento::nvm {

class LogChecker
    : public Checker<check::ASTDecl<FunctionDecl>, check::Bind,
                     check::ASTDecl<ValueDecl>, check::PostCall,
                     check::BeginFunction, check::BranchCondition> {
public:
  LogChecker() {}

  void checkBind(SVal Loc, SVal Val, const Stmt* S, CheckerContext& C) const;

  void checkPostCall(const CallEvent& Call, CheckerContext& C) const;

  void checkASTDecl(const FunctionDecl* FD, AnalysisManager& Mgr,
                    BugReporter& BR) const;

  void checkASTDecl(const ValueDecl* VD, AnalysisManager& Mgr,
                    BugReporter& BR) const;

  void checkBeginFunction(CheckerContext& Ctx) const;

  void checkBranchCondition(const Stmt* S, CheckerContext& C) const;

private:
};

} // namespace clang::ento::nvm
