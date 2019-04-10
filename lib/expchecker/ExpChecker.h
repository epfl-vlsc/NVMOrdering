#pragma once
#include "Common.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.expchecker";

namespace clang::ento::nvm {

class ExpChecker
    : public Checker<check::ASTDecl<TranslationUnitDecl>,
                     check::ASTDecl<RecordDecl>, check::BeginFunction,
                     check::PreCall, check::Bind, check::EndFunction> {

public:
  ExpChecker() {}

  void checkBeginFunction(CheckerContext& Ctx) const;

  void checkEndFunction(CheckerContext& C) const;

  void checkPreCall(const CallEvent& Call, CheckerContext& C) const;

  void checkBind(SVal Loc, SVal Val, const Stmt* S, CheckerContext& C) const;

  void checkASTDecl(const TranslationUnitDecl* CTUD, AnalysisManager& Mgr,
                    BugReporter& BR) const;

  void checkASTDecl(const RecordDecl* RD, AnalysisManager& Mgr,
                                BugReporter& BR) const;

private:
};

} // namespace clang::ento::nvm
