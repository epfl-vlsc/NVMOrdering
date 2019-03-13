#pragma once
#include "OrderingBugReporter.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerRegistry.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "llvm/Support/raw_ostream.h"
#include <utility>

constexpr const char *CHECKER_PLUGIN_NAME = "nvm.orderingchecker";

namespace clang::ento::nvm {

class OrderingChecker
    : public Checker<check::EndAnalysis, 
                     check::BeginFunction,
                     check::Bind,
                     check::Location,
                     check::PreCall,
                     check::BranchCondition> {

public:
  OrderingChecker();

  void checkBeginFunction(CheckerContext &Ctx) const;

  void checkEndAnalysis(ExplodedGraph &G, BugReporter &BR,
                        ExprEngine &Eng) const;

  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;

  void checkBind(SVal Loc, SVal Val, const Stmt *S, CheckerContext &C) const;

  void checkLocation(SVal Loc, bool IsLoad, const Stmt *S,
                     CheckerContext &C) const;

  void checkBranchCondition(const Stmt *Condition, CheckerContext &C) const;

private:
  OrderingBugReporter BReporter;
};

} // namespace clang::ento::nvm
