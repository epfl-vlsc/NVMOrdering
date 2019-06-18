#pragma once
#include "Common.h"

namespace clang::ento::nvm {

template <typename Globals, typename StmtParser, typename BReporter,
          typename Transitions>
class DataflowManager {
public:
  using LatVar = typename Transitions::LatVar;
  using AbstractState = typename Transitions::AbstractState;
  using FunctionResults = typename Transitions::FunctionResults;
  using DataflowResults = typename Transitions::DataflowResults;

private:
  // useful structures
  AnalysisManager& Mgr;

  Globals globals;
  StmtParser stmtParser;
  BReporter bugReporter;
  Transitions transitions;

public:
  DataflowManager(AnalysisManager& Mgr_, BugReporter& BR, const CheckerBase* CB)
      : Mgr(Mgr_), stmtParser(globals), bugReporter(globals, Mgr_, BR, CB),
        transitions(globals, stmtParser, bugReporter) {}

  void initUnit(const FunctionDecl* FD) {
    globals.setActiveUnit(FD);
    bugReporter.initUnit();
    globals.dumpUnit();
  }

  void initLatticeValues(AbstractState& state) {
    transitions.initLatticeValues(state);
  }

  // access structures
  auto& getAnalyzedFunctions() { return globals.getAnalyzedFunctions(); }

  auto& getGlobals() { return globals; }

  void reportBugs() const { bugReporter.reportBugs(); }

  bool handleStmt(const Stmt* S, AbstractState& state) {
    return transitions.handleStmt(S, state);
  }

  const CallExpr* getCEIfAnalyzedFunction(const Stmt* S) const {
    return globals.getCEIfAnalyzedFunction(S);
  }

  auto& getMgr() { return Mgr; }
};

} // namespace clang::ento::nvm
