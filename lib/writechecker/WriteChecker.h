#pragma once
#include "Common.h"
#include "DataInfos.h"
#include "FunctionInfos.h"
#include "States.h"
#include "Transitions.h"
#include "TypeInfos.h"
#include "WriteBugReporter.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.writechecker";

namespace clang::ento::nvm {

class WriteChecker
    : public Checker</*check::BeginFunction, check::EndFunction, check::Bind,
                     check::PreCall, check::ASTDecl<FunctionDecl>,
                     check::ASTDecl<DeclaratorDecl>, check::DeadSymbols,
                     check::PointerEscape*/
                     check::ASTDecl<TranslationUnitDecl>> {

public:
  WriteChecker() /*: BReporter(*this), nvmFncInfo("PersistentCode") */ {}

  /*
    void checkBeginFunction(CheckerContext& Ctx) const;

    void checkEndFunction(CheckerContext& C) const;

    void checkPreCall(const CallEvent& Call, CheckerContext& C) const;

    void checkBind(SVal Loc, SVal Val, const Stmt* S, CheckerContext& C) const;

    void checkASTDecl(const FunctionDecl* D, AnalysisManager& Mgr,
                      BugReporter& BR) const;

    void checkASTDecl(const DeclaratorDecl* D, AnalysisManager& Mgr,
                      BugReporter& BR) const;

    void checkDeadSymbols(SymbolReaper& SymReaper, CheckerContext& C) const;

    ProgramStateRef checkPointerEscape(ProgramStateRef State,
                                       const InvalidatedSymbols& Escaped,
                                       const CallEvent* Call,
                                       PointerEscapeKind Kind) const;
  */
  void checkASTDecl(const TranslationUnitDecl* TUD, AnalysisManager& Mgr,
                    BugReporter& BR) const;

private:
  /*

  void handleFlush(const CallEvent& Call, CheckerContext& C) const;

  void handlePFence(const CallEvent& Call, CheckerContext& C) const;

  void handleVFence(const CallEvent& Call, CheckerContext& C) const;

  void handleWriteData(CheckerContext& C, const DeclaratorDecl* D,
                       DataInfo* DI) const;

  void handleWriteCheck(SVal Loc, CheckerContext& C, const DeclaratorDecl* D,
                        CheckInfo* CI) const;

  void handleWriteMask(SVal Loc, const Stmt* S, CheckerContext& C,
                       const DeclaratorDecl* D, CheckDataInfo* DCI) const;

  void handleFlushData(CheckerContext& C, const DeclaratorDecl* D,
                       DataInfo* DI) const;

  void handleFlushCheck(CheckerContext& C, const DeclaratorDecl* D,
                        CheckInfo* CI) const;

  WriteBugReporter BReporter;
  mutable NVMFunctionInfo nvmFncInfo;
  mutable NVMTypeInfo nvmTypeInfo;
  */
  mutable VarInfos varInfos;
};

} // namespace clang::ento::nvm
