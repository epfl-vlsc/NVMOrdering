#pragma once
#include "Common.h"
#include "DfUtil.h"
#include "DsclState.h"
#include "ProgramLocation.h"

namespace clang::ento::nvm {

template <typename TrackVar> class DataFlow {
  // variables to track
  using TrackVars = std::set<TrackVar>;

  // result types
  using AbstractState = std::map<const NamedDecl*, DsclValue>;
  using FunctionResults = std::map<ProgramLocation, AbstractState>;
  using AllResults = std::map<PlContext, FunctionResults>;

  // context helpers
  using PlContextSet = std::set<PlContext>;
  using PlContextMap = std::map<PlContext, PlContextSet>;

  // worklist
  template <typename WorkElement>
  using Worklist = SmallVector<WorkElement, 200>;
  using ContextWorklist = Worklist<PlContext>;
  using BlockWorklist = Worklist<const CFGBlock*>;

  // data structures
  AllResults allResults;
  ContextWorklist contextWork;
  PlContextMap callers;
  PlContextSet active;

  // top info
  const CFG* topFunction;
  const TrackVars& trackVars;
  AnalysisManager& mgr;

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

  FunctionResults& initFunctionResults(const CFG* function,
                                       PlContext& context) {
    FunctionResults& functionResults = allResults[context];

    // initialize entry
    ProgramLocation entryBlock = Forward::getEntryBlock(function);
    AbstractState& state = functionResults[entryBlock];
    for (TrackVar trackVar : trackVars) {
      state[trackVar] = DsclValue::getInit(trackVar.isDcl, trackVar.isScl);
    }

    return functionResults;
  }

  void addBlocksToWorklist(const BlockWorklist& blockWorkList,
                           const CFG* function) {
    for (const CFGBlock* block : Forward::getBlocks(function)) {
      blockWorkList.push_back(block);
    }
  }

  void applyTransfer(const Stmt* S, AbstractState& state) {}

  void mergePrevStates(const CFGBlock* block, FunctionResults& results) {}

  void analyzeCall(const CallExpr* CE, AbstractState& state,
                   const PlContext& context) {}

  void computeDataflow(const CFG* function, PlContext& context) {
    active.insert({context, function});

    // initialize results
    FunctionResults& results = initFunctionResults(function, context);

    // initialize worklist
    BlockWorklist blockWorklist;
    addBlocksToWorklist(blockWorklist, function);

    while (!blockWorklist.empty()) {
      const CFGBlock* block = worklist.pop_back_val();
      ProgramLocation plBlock(block);

      // get previously computed states
      AbstractState& oldEntryState = results[Forward::getEntryKey(block)];
      AbstractState& oldExitState = results[Forward::getExitKey(block)];

      // get current state
      AbstractState& state = mergePrevStates(block, results);
      // todo summary key

      // skip block if same result
      if (state == oldEntryState && !state.empty()) {
        continue;
      }

      // update based on predecessors
      results[plBlock] = state;

      


    }
  }

  AllResults analyze() {}

public:
  DataFlow(const CFG* function, const TrackedVars& trackedVars_,
           AnalysisManager& mgr_)
      : topFunction(function), trackedVars(trackedVars_), mgr(mgr_) {
    contextWork.emplace_back(PlContext(function));
  }

  AllResults computeDataFlow() {
    while (!contextWork.empty()) {
      auto [context, function] = contextWork.pop_back_val();
      computeDataflow(function, context);
    }

    return allResults;
  }
};

} // namespace clang::ento::nvm