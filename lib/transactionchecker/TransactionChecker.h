#pragma once
#include "Common.h"
#include "States.h"
#include "TransactionBugReporter.h"
#include "TransactionInfos.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.transactionchecker";

namespace clang::ento::nvm {

class TransactionChecker
    : public Checker<check::BeginFunction, check::Bind,
                     check::ASTDecl<FunctionDecl>, check::DeadSymbols,
                     check::PointerEscape, check::PostCall> {
public:
  TransactionChecker() : BReporter(*this) {}

  void checkBeginFunction(CheckerContext& C) const;

  void checkBind(SVal Loc, SVal Val, const Stmt* S, CheckerContext& C) const;

  void checkPostCall(const CallEvent& Call, CheckerContext& C) const;

  void checkASTDecl(const FunctionDecl* D, AnalysisManager& Mgr,
                    BugReporter& BR) const;

  void checkDeadSymbols(SymbolReaper& SymReaper, CheckerContext& C) const;

  ProgramStateRef checkPointerEscape(ProgramStateRef State,
                                     const InvalidatedSymbols& Escaped,
                                     const CallEvent* Call,
                                     PointerEscapeKind Kind) const;

private:
  void handleTxBegin(CheckerContext& C) const;

  void handleTxEnd(CheckerContext& C) const;

  TransactionBugReporter BReporter;
  mutable NVMTransactionInfo nvmTxInfo;
};

} // namespace clang::ento::nvm
