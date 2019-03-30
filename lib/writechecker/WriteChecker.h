#pragma once
#include "AstWalkers.h"
#include "Common.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.writechecker";

namespace clang::ento::nvm {

/*
              , check::DeadSymbols,
              check::PointerEscape*/
/*,*/

class WriteChecker
    : public Checker<check::ASTDecl<TranslationUnitDecl>, check::BeginFunction,
                     check::PreCall, check::Bind, check::EndFunction> {

public:
  WriteChecker()
      : BReporter(*this), fncInfos("PersistentCode") {}

  void checkBeginFunction(CheckerContext& Ctx) const;

  void checkEndFunction(CheckerContext& C) const;

  void checkPreCall(const CallEvent& Call, CheckerContext& C) const;

  void checkBind(SVal Loc, SVal Val, const Stmt* S, CheckerContext& C) const;
  /*

      void checkDeadSymbols(SymbolReaper& SymReaper, CheckerContext& C) const;

      ProgramStateRef checkPointerEscape(ProgramStateRef State,
                                         const InvalidatedSymbols& Escaped,
                                         const CallEvent* Call,
                                         PointerEscapeKind Kind) const;
    */
  void checkASTDecl(const TranslationUnitDecl* CTUD, AnalysisManager& Mgr,
                    BugReporter& BR) const;

private:
  void addStateTransition(ProgramStateRef& State, CheckerContext& C,
                          bool stateChanged) const;

  void handleFlush(const CallEvent& Call, CheckerContext& C) const;

  template <bool pfence>
  void handleFence(const CallEvent& Call, CheckerContext& C) const;

  template <typename SMap>
  void checkMapStates(ProgramStateRef& State, CheckerContext& C) const;

  template <typename SMap, bool pfence>
  void checkFenceStates(ProgramStateRef& State, CheckerContext& C,
                         bool& stateChanged) const;

  template <typename SMap>
  void printStates(ProgramStateRef& State, CheckerContext& C) const;

  /*

  void handleWriteData(CheckerContext& C, const DeclaratorDecl* D,
                       DataInfo* DI) const;

  void handleWriteCheck(SVal Loc, CheckerContext& C, const DeclaratorDecl*
  D, CheckInfo* CI) const;

  void handleWriteMask(SVal Loc, const Stmt* S, CheckerContext& C,
                       const DeclaratorDecl* D, CheckDataInfo* DCI) const;

  void handleFlushData(CheckerContext& C, const DeclaratorDecl* D,
                       DataInfo* DI) const;

  void handleFlushCheck(CheckerContext& C, const DeclaratorDecl* D,
                        CheckInfo* CI) const;



  */

  const WriteBugReporter BReporter;
  mutable FunctionInfos fncInfos;
  mutable VarInfos varInfos;
};

} // namespace clang::ento::nvm
