#pragma once
#include "Common.h"
#include "DfUtil.h"
#include "DsclState.h"
#include "ProgramLocation.h"

namespace clang::ento::nvm {

class DataFlow {
  // variables to track
  using TrackedVars = std::set<const NamedDecl*>;

  // result types
  using AbstractState = std::map<const NamedDecl*, DsclValue>;
  using FunctionResults = std::map<ProgramLocation, AbstractState>;
  using AllResults = std::map<PlContext, FunctionResults>;

  // context helpers
  using PlContextSet = std::set<PlContext>;
  using PlContextMap = std::map<PlContext, PlContextSet>;

  // worklist
  template <typename WorkElement> using Worklist = SmallVector<WorkElement, 20>;
  using ContextWorklist = Worklist<PlContext>;
  using BlockWorklist = Worklist<const CFGBlock*>;

  // data structures
  AllResults allResults;
  ContextWorklist contextWork;
  PlContextMap callers;
  PlContextSet active;

  void dump() {
    for (auto& [context, results] : allResults) {
      context.dump();
      for (auto& [pl, states] : results) {
        pl.dump();
        for (auto& [ND, LV] : states) {
          printTrackedVar(ND, LV);
          
        }
      }
    }
  }

  // top info
  TrackedVars trackedVars;

  FunctionResults& initFunctionResults(const CFG* function,
                                       PlContext& context) {
    FunctionResults& functionResults = allResults[context];

    // initialize entry
    const CFGBlock* entryBlock = Forward::getEntryBlock(f);
    AbstractState& state = functionResults[ProgramLocation(entryBlock)];
    state =

        return functionResults;
  }

public:
  DataFlow(const CFG* function, const TrackedVars& trackedVars_)
      : trackedVars(trackedVars_) {
    contextWork.emplace_back(PlContext(function));
  }

  void applyTransfer(const Stmt* S, AbstractState& state) {}

  void mergePrevStates(const CFGBlock* block, FunctionResults& results) {}

  void analyzeCall(const CallExpr* CE, AbstractState& state,
                   const PlContext& context) {}

  AllResults computeDataFlow() {
    while (!contextWork.empty()) {
      auto [context, function] = contextWork.pop_back_val();
      computeDataflow(function, context);
    }

    return allResults;
  }

  void computeDataflow(const CFG* function, PlContext& context) {
    active.insert({context, function});

    FunctionResults& results = initFunctionResults(function, context);
  }

  AllResults analyze() {}
};

} // namespace clang::ento::nvm