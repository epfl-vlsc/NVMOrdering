#pragma once
#include "Common.h"
#include "AstWalkers.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.writechecker";

namespace clang::ento::nvm {

/*,check::EndFunction
              check::PreCall, check::DeadSymbols,
              check::PointerEscape*/
/*,*/

class WriteChecker : public Checker<check::ASTDecl<TranslationUnitDecl>,
                                    check::BeginFunction, check::Bind> {

public:
  WriteChecker()
      : BReporter(*this), ErrNode(nullptr), fncInfos("PersistentCode") {}

  void checkBeginFunction(CheckerContext& Ctx) const;
  /*
    void checkEndFunction(CheckerContext& C) const;

    void checkPreCall(const CallEvent& Call, CheckerContext& C) const;
*/

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
  void addStateTransition(CheckerContext& C, bool stateChanged) const;

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



  */

  const WriteBugReporter BReporter;
  mutable ExplodedNode* ErrNode;
  mutable FunctionInfos fncInfos;
  mutable VarInfos varInfos;
};

} // namespace clang::ento::nvm
