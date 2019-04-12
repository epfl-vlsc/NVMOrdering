#pragma once
#include "ReadWalkers.h"
#include "Common.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.readchecker";

namespace clang::ento::nvm {

class ReadChecker : public Checker<check::ASTDecl<TranslationUnitDecl>,
                                   check::BeginFunction, check::Location> {
public:
  ReadChecker() : BReporter(*this) {}

  void checkBeginFunction(CheckerContext& Ctx) const;

  void checkLocation(SVal Loc, bool IsLoad, const Stmt* S,
                     CheckerContext& C) const;

  void checkASTDecl(const TranslationUnitDecl* CTUD, AnalysisManager& Mgr,
                    BugReporter& BR) const;

private:
  void addStateTransition(ProgramStateRef& State, CheckerContext& C,
                         bool stateChanged) const;

  void handleEnd(CheckerContext& C) const;

  ReadBugReporter BReporter;
  mutable OrderFncs orderFncs;
  mutable OrderVars orderVars;
};

} // namespace clang::ento::nvm
