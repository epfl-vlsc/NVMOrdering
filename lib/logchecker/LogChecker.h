// transaction pmdk checker
#pragma once
#include "Common.h"
#include "LogBugReporter.h"
#include "LogInfos.h"
#include "States.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.logchecker";

namespace clang::ento::nvm {

class LogChecker : public Checker<check::ASTDecl<FunctionDecl>, check::Bind,
                                  check::ASTDecl<ValueDecl>, check::PostCall> {
public:
  LogChecker() : BReporter(*this) {}

  void checkBind(SVal Loc, SVal Val, const Stmt* S, CheckerContext& C) const;

  void checkPostCall(const CallEvent& Call, CheckerContext& C) const;

  void checkASTDecl(const FunctionDecl* FD, AnalysisManager& Mgr,
                    BugReporter& BR) const;

  void checkASTDecl(const ValueDecl* VD, AnalysisManager& Mgr,
                    BugReporter& BR) const;

private:
  void handlePtrFnc(const CallEvent& Call, CheckerContext& C) const;

  void handleCodeFnc(const CallEvent& Call, CheckerContext& C) const;

  LogBugReporter BReporter;
  mutable LogInfos logInfos;
};

} // namespace clang::ento::nvm
