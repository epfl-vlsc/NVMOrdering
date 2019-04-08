#pragma once
#include "Common.h"
#include "DbgState.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.mcchecker";

namespace clang::ento::nvm {

class MultiCompileChecker : public Checker<check::PreCall, check::Bind> {

public:
  MultiCompileChecker() {
    bug.reset(new BugType(this, "bug", "bug"));
  }
  void checkPreCall(const CallEvent& Call, CheckerContext& C) const;

  void checkBind(SVal Loc, SVal Val, const Stmt* S, CheckerContext& C) const;

  mutable std::unique_ptr<BugType> bug;
};


} // namespace clang::ento::nvm
