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
  using FunctionContext = std::pair<const FunctionDecl*, PlContext>;
  using FunctionContextSet = std::set<FunctionContext>;
  using FunctionContextMap = std::map<FunctionContext, FunctionContextSet>;

  // worklist
  template <typename WorkElement, int N>
  using Worklist = SmallVector<WorkElement, N>;
  using ContextWorklist = Worklist<FunctionContext, 100>;
  using BlockWorklist = Worklist<const CFGBlock*, 200>;

  // data structures
  AllResults allResults;
  ContextWorklist contextWork;
  FunctionContextMap callers;
  FunctionContextSet active;

  // top info
  const FunctionDecl* topFunction;
  const TrackVars& trackVars;
  AnalysisManager& mgr;

  FunctionResults& initFunctionResults(const FunctionDecl* function,
                                       PlContext& context) {
    FunctionResults& functionResults = allResults[context];

    // initialize blocks
    const CFG* cfg = mgr.getCFG(function);
    for (const CFGBlock* block : Forward::getRBlocks(cfg)) {
      ProgramLocation entryKey = Forward::getEntryKey(block);
      ProgramLocation exitKey = Forward::getExitKey(block);

      AbstractState& entryState = functionResults[entryKey];
      AbstractState& exitState = functionResults[exitKey];

      for (TrackVar trackVar : trackVars) {
        const NamedDecl* var = trackVar.ND;
        auto lv = DsclValue::getInit(trackVar.isDcl, trackVar.isScl);
        entryState[var] = lv;
        exitState[var] = lv;
      }
    }

    return functionResults;
  }

  void addBlocksToWorklist(BlockWorklist& blockWorkList,
                           const FunctionDecl* function) {
    const CFG* cfg = mgr.getCFG(function);
    for (const CFGBlock* block : Forward::getBlocks(cfg)) {
      blockWorkList.push_back(block);
    }
  }

  void applyTransfer(const Stmt* S, AbstractState& state) {}

  AbstractState mergePrevStates(const ProgramLocation& entryKey,
                                FunctionResults& results) {
    AbstractState mergedState;
    AbstractState& inState = results[entryKey];

    // start with current entry
    mergeInState(mergedState, inState);

    // get all prev blocks
    for (const CFGBlock* pred_block : Forward::getPredecessorBlocks(entryKey)) {
      ProgramLocation exitKey = Forward::getExitKey(pred_block);
      if (results.count(exitKey)) {
        AbstractState& predecessorState = results[exitKey];
        mergeInState(mergedState, predecessorState);
      }
    }
    return mergedState;
  }

  void mergeInState(AbstractState& out, AbstractState& in) {
    for (auto& [inVar, inVal] : in) {
      if (out.count(inVar)) {
        // if var exists meet
        auto& outVal = out[inVar];
        auto meetVal = outVal.meet(inVal);
        out[inVar] = meetVal;
      } else {
        // if var does not exist copy
        out[inVar] = inVal;
      }
    }
  }

  void analyzeCall(const CallExpr* CE, AbstractState& state,
                   const PlContext& context) {}

  void analyzeStmts(const CFGBlock* block, AbstractState& state,
                    FunctionResults& results, const PlContext& context) {
    /*
    for (const CFGElement element : Forward::getElements(block)) {
      printMsg("lol");
      if (Optional<CFGStmt> CS = element.getAs<CFGStmt>()) {
        const Stmt* S = CS->getStmt();
        printStmt(S, mgr, "s");
        printStmt(S, "s");
      }
    }
    */
  }

  void computeDataflow(const FunctionDecl* function, PlContext& context) {
    active.insert({function, context});

    // initialize results
    FunctionResults& results = initFunctionResults(function, context);
    // todo summary key

    // initialize worklist
    BlockWorklist blockWorklist;
    addBlocksToWorklist(blockWorklist, function);

    while (!blockWorklist.empty()) {
      const CFGBlock* block = blockWorklist.pop_back_val();
      ProgramLocation entryKey = Forward::getEntryKey(block);
      ProgramLocation exitKey = Forward::getExitKey(block);

      // get previously computed states
      AbstractState& oldEntryState = results[entryKey];
      AbstractState& oldExitState = results[exitKey];

      // get current state
      AbstractState state = mergePrevStates(entryKey, results);

      // todo summary key

      // skip block if same result
      if (state == oldEntryState) {
        continue;
      }

      // update entry based on predecessors
      results[entryKey] = state;

      // todo go over statements
      analyzeStmts(block, state, results, context);

      // skip if state not updated
      if (state == oldExitState) {
        continue;
      }

      // update final state
      results[exitKey] = state;

      // do data flow for successive blocks
      for (const CFGBlock* succBlock : Forward::getSuccessorBlocks(block)) {
        blockWorklist.push_back(succBlock);
      }

      // todo summary
    }

    // data flow over all blocks for the function has finished
    // update context information
    // if function changed update all callers
    FunctionResults& oldResults = allResults[context];
    if (oldResults != results) {
      oldResults = results;
      for (const FunctionContext& caller : callers[{function, context}]) {
        contextWork.push_back(caller);
      }
    }

    // printFunctionResults(results);
    active.erase({function, context});
  }

  AllResults analyze() {}

public:
  DataFlow(const FunctionDecl* function, const TrackVars& trackVars_,
           AnalysisManager& mgr_)
      : topFunction(function), trackVars(trackVars_), mgr(mgr_) {
    contextWork.push_back({function, PlContext()});
  }

  AllResults computeDataFlow() {
    while (!contextWork.empty()) {
      auto [function, context] = contextWork.pop_back_val();
      computeDataflow(function, context);
    }
    return allResults;
  }

  void dumpAS(const AbstractState& state) const {
    for (auto& [ND, LV] : state) {
      printTrackedVar(ND, LV, false);
      printMsg(" - ", false);
    }
    printMsg("");
  }

  void dumpFR(const FunctionResults& results) const {
    for (auto& [pl, state] : results) {
      pl.dump(mgr);
      dumpAS(state);
    }
  }

  void dump() const {
    printMsg("-------All results------");
    for (auto& [context, results] : allResults) {
      context.dump();
      dumpFR(results);
    }
  }
};

} // namespace clang::ento::nvm