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
void dumpFunctionResults(const FunctionResults& results, AnalysisManager& Mgr) {
  for (auto& [pl, state] : results) {
    pl.dump(Mgr);
    llvm::errs() << "--> ";
    dumpAbstractState(state);
  }
}

template <typename DataflowResults>
void dumpDataflowResults(const DataflowResults& dataflowResults,
                         AnalysisManager& Mgr) {
  llvm::errs() << "---All results--- no of contexts(" << dataflowResults.size() << ")\n";
  for (auto& [context, results] : dataflowResults) {
    context.dump(Mgr);
    dumpFunctionResults(results, Mgr);
  }
}

}