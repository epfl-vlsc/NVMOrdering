#pragma once
#include "Common.h"
#include "FunctionInfos.h"
#include "ReadBugReporter.h"
#include "States.h"
#include "Transitions.h"
#include "TypeInfos.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.readchecker";

namespace clang::ento::nvm {

class ReadChecker
    : public Checker<check::BeginFunction, check::Location,
                     check::ASTDecl<FunctionDecl>,
                     check::ASTDecl<DeclaratorDecl>, check::DeadSymbols,
                     check::PointerEscape> {
public:
  ReadChecker() : BReporter(*this), nvmFncInfo("RecoveryCode") {}

  void checkBeginFunction(CheckerContext& Ctx) const;

  void checkLocation(SVal Loc, bool IsLoad, const Stmt* S,
                     CheckerContext& C) const;

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
  void handleReadData(SVal Loc, CheckerContext& C, const DeclaratorDecl* DD,
                      DataInfo* DI) const;

  void handleReadCheck(CheckerContext& C, const DeclaratorDecl* DD,
                       CheckInfo* CI) const;

  void handleReadMask(SVal Loc, const Stmt* S, CheckerContext& C,
                      const DeclaratorDecl* DD, CheckDataInfo* CDI) const;

  ReadBugReporter BReporter;
  mutable NVMFunctionInfo nvmFncInfo;
  mutable NVMTypeInfo nvmTypeInfo;
};

} // namespace clang::ento::nvm
