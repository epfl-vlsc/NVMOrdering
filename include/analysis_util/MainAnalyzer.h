#pragma once
#include "Common.h"
#include "dataflow_util/DataflowAnalysis.h"

namespace clang::ento::nvm {

template <typename Manager, typename Parser> class MainAnalyzer {
protected:
  void parseTUD(Manager& manager, TranslationUnitDecl* TUD,
                AnalysisManager& Mgr) {
    auto& AC = Mgr.getASTContext();
    auto& globals = manager.getGlobals();

    // parse entire translation unit
    Parser parser(globals, AC);
    parser.TraverseDecl(TUD);
    parser.fillStructures();

    globals.dump();
  }

  void doDataflowFD(const FunctionDecl* FD, Manager& manager) {
    printND(FD, "***analyzing function***");
    // run data flow analysis
    DataflowAnalysis dataflowAnalysis(FD, manager);
    dataflowAnalysis.computeDataflow();

    // report bugs
    dataflowAnalysis.reportBugs();
  }

  void doDataflowTUD(Manager& manager) {
    for (const FunctionDecl* FD : manager.getAnalyzedFunctions()) {
      manager.initUnit(FD);

      // run data flow on a function inter-procedurally
      doDataflowFD(FD, manager);
    }
  }

public:
  void analyzeTUD(TranslationUnitDecl* TUD, AnalysisManager& Mgr,
                  BugReporter& BR, const CheckerBase* CB) {
    // init program helper
    Manager manager(Mgr, BR, CB);

    // parse entire program
    parseTUD(manager, TUD, Mgr);

    // run data flow analysis on units
    doDataflowTUD(manager);
  }
};

} // namespace clang::ento::nvm