#pragma once
#include "Common.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.expchecker";

namespace clang::ento::nvm {

class ExpChecker : public Checker<check::Bind,
                                  check::BranchCondition, 
                                  check::PostCall,
                                  check::PreCall> {

public:
  ExpChecker() { bug.reset(new BugType(this, "bug", "bug")); }
  void checkPreCall(const CallEvent& Call, CheckerContext& C) const;

  void checkBind(SVal Loc, SVal Val, const Stmt* S, CheckerContext&) const;

  void checkBranchCondition(const Stmt* Condition, CheckerContext& C) const;

  void checkPostCall(const CallEvent& Call, CheckerContext& C) const;

  mutable std::unique_ptr<BugType> bug;
};

} // namespace clang::ento::nvm
