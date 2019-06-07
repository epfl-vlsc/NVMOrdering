#pragma once
#include "Common.h"
#include "DataflowAnalysis.h"

namespace clang::ento::nvm {

template <typename Vars, typename Functions, typename LatVal, typename SubClass>
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

  // convenient data structure per unit analysis
  FunctionInfo* activeUnitInfo;
  DataflowResults* dataflowResults;

  void parseTUD(TranslationUnitDecl* TUD) {
    ASTContext& astContext = Mgr->getASTContext();
    PairParser pairParser(vars, funcs, astContext);
    pairParser.TraverseDecl(TUD);
    pairParser.fillStructures();
    // pairParser.createGraphs();

    funcs.dump();
    vars.dump();
  }

  void doDataflowFD(const FunctionDecl* FD) {
    printND(FD, "***analyzing function***");

    // get unit info
    activeUnitInfo = &funcs.getUnitInfo(FD);
    activeUnitInfo->dump();

    // run data flow analysis
    DataflowAnalysis dataflowAnalysis(FD, getAnalyzer());
    // dataflowResults = &dataflowAnalysis.computeDataflow();
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

  virtual SubClass& getAnalyzer() = 0;
  virtual void initLatticeValues(AbstractState& state) = 0;
};

} // namespace clang::ento::nvm