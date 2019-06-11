#pragma once
#include "Common.h"

namespace clang::ento::nvm {

template <typename AbstractState>
void dumpAbstractState(const AbstractState& state) {
  for (auto& [ND, LV] : state) {
    printND(ND, "Var", true, false);
    LV.dump();
    llvm::errs() << " - ";
  }
  llvm::errs() << "\n";
}

template <typename FunctionResults>
void dumpFunctionResults(const FunctionResults& results, AnalysisManager* Mgr) {
  for (auto& [al, state] : results) {
    al->dump();
    dumpAbstractState(state);
  }
}

template <typename DataflowResults>
void dumpDataflowResults(const DataflowResults* dataflowResults,
                         AnalysisManager* Mgr) {
  printMsg("---All results---");
  for (auto& [context, results] : *dataflowResults) {
    context.fullDump(Mgr);
    dumpFunctionResults(results, Mgr);
  }
}

}