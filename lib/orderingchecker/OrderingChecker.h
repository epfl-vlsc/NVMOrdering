#pragma once
#include "OrderingBugReporter.h"
#include "Common.h"
#include "FunctionInfos.h"
#include "TypeInfos.h"
#include "States.h"
#include "Transitions.h"

constexpr const char *CHECKER_PLUGIN_NAME = "nvm.orderingchecker";

namespace clang::ento::nvm
{

class OrderingChecker
    : public Checker<check::BeginFunction, check::EndFunction,
                     check::Bind, check::PreCall,
                     check::ASTDecl<FunctionDecl>, check::ASTDecl<DeclaratorDecl>,
                     check::DeadSymbols, check::PointerEscape>
{

public:
  OrderingChecker() : BReporter(*this) {}

  void checkBeginFunction(CheckerContext &Ctx) const;

  void checkEndFunction(CheckerContext &C) const;

  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;

  void checkBind(SVal Loc, SVal Val, const Stmt *S, CheckerContext &C) const;

  void checkASTDecl(const FunctionDecl *D, AnalysisManager &Mgr,
                    BugReporter &BR) const;

  void checkASTDecl(const DeclaratorDecl *D, AnalysisManager &Mgr,
                    BugReporter &BR) const;

  void checkDeadSymbols(SymbolReaper &SymReaper, CheckerContext &C) const;

  ProgramStateRef checkPointerEscape(ProgramStateRef State,
                                     const InvalidatedSymbols &Escaped,
                                     const CallEvent *Call,
                                     PointerEscapeKind Kind) const;

private:
  void handleFlush(const CallEvent &Call, CheckerContext &C) const;

  void handlePFence(const CallEvent &Call, CheckerContext &C) const;

  void handleVFence(const CallEvent &Call, CheckerContext &C) const;

  void handleWriteData(CheckerContext &C, const DeclaratorDecl *D,
                       DataInfo *DI) const;

  void handleWriteCheck(CheckerContext &C, const DeclaratorDecl *D,
                        CheckInfo *CI) const;

  void handleWriteMask(const Stmt *S, CheckerContext &C, const DeclaratorDecl *D,
                                        CheckDataInfo *DCI) const;

  void handleFlushData(CheckerContext &C, const DeclaratorDecl *D,
                       DataInfo *DI) const;

  void handleFlushCheck(CheckerContext &C, const DeclaratorDecl *D,
                        CheckInfo *CI) const;

  OrderingBugReporter BReporter;
  mutable NVMFunctionInfo nvmFncInfo;
  mutable NVMTypeInfo nvmTypeInfo;
};

} // namespace clang::ento::nvm


