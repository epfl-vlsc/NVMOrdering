#pragma once
#include "OrderingBugReporter.h"
#include "Common.h"
#include "FunctionInfos.h"

constexpr const char *CHECKER_PLUGIN_NAME = "nvm.orderingchecker";

namespace clang::ento::nvm
{

class OrderingChecker
    : public Checker<check::EndAnalysis, check::BeginFunction, check::Bind,
                     check::PreCall, check::BranchCondition,
                     check::ASTDecl<FunctionDecl>>
{

public:
  OrderingChecker();

  void checkBeginFunction(CheckerContext &Ctx) const;

  void checkEndAnalysis(ExplodedGraph &G, BugReporter &BR,
                        ExprEngine &Eng) const;

  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;

  void checkBind(SVal Loc, SVal Val, const Stmt *S, CheckerContext &C) const;

  void checkBranchCondition(const Stmt *Condition, CheckerContext &C) const;

  void checkEndOfTranslationUnit(const TranslationUnitDecl *TU,
                                 AnalysisManager &Mgr, BugReporter &BR) const;

  void checkASTDecl(const FunctionDecl *D, AnalysisManager &Mgr,
                    BugReporter &BR) const;

private:
  OrderingBugReporter BReporter;
  mutable NVMFunctionInfo nvmFncInfo;
};

} // namespace clang::ento::nvm
