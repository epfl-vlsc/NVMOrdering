#pragma once
#include "Common.h"
#include "DataflowAnalysis.h"

namespace clang::ento::nvm {

template <typename Vars, typename Functions, typename LatVal,
          typename Transitions, typename SubClass>
class MainAnalyzer {
public:
  using FunctionInfo = typename Functions::FunctionInfo;
  using LatVar = const NamedDecl*;
  using AbstractState = std::map<LatVar, LatVal>;
  using FunctionResults = std::map<ProgramLocation, AbstractState>;
  using DataflowResults = std::map<PlContext, FunctionResults>;

protected:
  // program helpers
  AnalysisManager* Mgr;
  BugReporter* BR;

  // whole program data structures
  Vars vars;
  Functions funcs;
  Transitions transitions;

  // convenient data structure per unit analysis
  FunctionInfo* activeUnitInfo;
  DataflowResults* dataflowResults;

  void parseTUD(TranslationUnitDecl* TUD) {
    ASTContext& astContext = Mgr->getASTContext();
    PairParser pairParser(vars, funcs, astContext);
    pairParser.TraverseDecl(TUD);
    pairParser.fillStructures();
    // pairParser.createGraphs();

    //initialize transitions
    transitions.initAll(vars, funcs, Mgr);
    funcs.dump();
    vars.dump();
  }

  void doDataflowFD(const FunctionDecl* FD) {
    printND(FD, "***analyzing function***");

    // get unit info and current transitions
    activeUnitInfo = &funcs.getUnitInfo(FD);
    activeUnitInfo->dump();
    transitions.initUnit(activeUnitInfo);

    // run data flow analysis
    DataflowAnalysis dataflowAnalysis(FD, getAnalyzer());
    dataflowResults = dataflowAnalysis.computeDataflow();
    dumpDataflowResults(dataflowResults, Mgr);
  }

  void doDataflowTUD() {
    for (const FunctionDecl* FD : funcs) {
      // run data flow on a function inter-procedurally
      doDataflowFD(FD);

      // report bugs
      reportBugs();
    }
  }

  void reportBugs() const {}

public:
  void analyzeTUD(TranslationUnitDecl* TUD, AnalysisManager& Mgr_,
                  BugReporter& BR_) {
    // init program helpers
    Mgr = &Mgr_;
    BR = &BR_;

    // parse entire program
    parseTUD(TUD);

    // run data flow analysis on units
    doDataflowTUD();
  }

  AnalysisManager* getMgr() { return Mgr; }

  bool isIpaCall(const Stmt* S) {
    if (const CallExpr* CE = dyn_cast<CallExpr>(S)) {
      const FunctionDecl* calleeFD = CE->getDirectCallee();
      if (!calleeFD)
        return false;

      return !funcs.isSkipFunction(calleeFD);
    }

    return false;
  }

  void initLatticeValues(AbstractState& state) {
    transitions.initLatticeValues(state);
  }

  bool handleStmt(const Stmt* S, AbstractState& state) {
    return transitions.handleStmt(S, state);
  }

  virtual SubClass& getAnalyzer() = 0;
};

} // namespace clang::ento::nvm