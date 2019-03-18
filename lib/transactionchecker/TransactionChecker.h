#pragma once
#include "Common.h"
#include "TransactionBugReporter.h"
#include "TransactionInfos.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.transactionchecker";

namespace clang::ento::nvm {

class TransactionChecker
    : public Checker<check::BeginFunction, check::Bind,
                     check::ASTDecl<FunctionDecl>,
                     check::ASTDecl<DeclaratorDecl>, check::DeadSymbols,
                     check::PointerEscape> {
public:
  TransactionChecker() : BReporter(*this) {}

  void checkBeginFunction(CheckerContext& Ctx) const;

  void checkBind(SVal Loc, SVal Val, const Stmt *S, CheckerContext &) const;

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

  TransactionBugReporter BReporter;
  mutable NVMTransactionInfo nvmTxInfo;
};

} // namespace clang::ento::nvm
