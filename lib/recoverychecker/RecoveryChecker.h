#pragma once
#include "Common.h"
#include "FunctionInfos.h"
#include "RecoveryBugReporter.h"
#include "States.h"
#include "Transitions.h"
#include "TypeInfos.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.recoverychecker";

namespace clang::ento::nvm {

class RecoveryChecker
    : public Checker<check::BeginFunction, check::EndFunction, check::Location,
                     check::ASTDecl<FunctionDecl>, check::BranchCondition,
                     check::ASTDecl<DeclaratorDecl>, check::DeadSymbols,
                     check::PointerEscape> {

public:
  RecoveryChecker() : BReporter(*this), nvmFncInfo("RecoveryCode") {}

  void checkBeginFunction(CheckerContext& Ctx) const;

  void checkEndFunction(CheckerContext& C) const;

  void checkLocation(SVal Loc, bool IsLoad, const Stmt* S,
                     CheckerContext&) const {}

  void checkBranchCondition(const Stmt* Condition, CheckerContext& Ctx) const {}

  void checkASTDecl(const FunctionDecl* D, AnalysisManager& Mgr,
                    BugReporter& BR) const;

  void checkASTDecl(const DeclaratorDecl* D, AnalysisManager& Mgr,
                    BugReporter& BR) const;

  void checkDeadSymbols(SymbolReaper& SymReaper, CheckerContext& C) const;

  ProgramStateRef checkPointerEscape(ProgramStateRef State,
                                     const InvalidatedSymbols& Escaped,
                                     const CallEvent* Call,
                                     PointerEscapeKind Kind) const;

private:
  RecoveryBugReporter BReporter;
  mutable NVMFunctionInfo nvmFncInfo;
  mutable NVMTypeInfo nvmTypeInfo;
};

} // namespace clang::ento::nvm
