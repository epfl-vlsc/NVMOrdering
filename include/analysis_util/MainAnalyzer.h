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
    Parser parser(TUD, globals, AC);
    globals.dump();
  }

  void doDataflowFD(const FunctionDecl* FD, Manager& manager) {
    
    

    // run data flow analysis
    DataflowAnalysis dataflowAnalysis(FD, manager);
    dataflowAnalysis.computeDataflow();

    // report bugs
    dataflowAnalysis.reportBugs();
  }

  void doDataflowTUD(Manager& manager) {
    for (const FunctionDecl* FD : manager.getAnalyzedFunctions()) {
      printND(FD, "***analyzing function***");
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