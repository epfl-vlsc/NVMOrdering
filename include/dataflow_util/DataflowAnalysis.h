#pragma once
#include "AbstractProgram.h"
#include "Common.h"
#include "DataflowPrint.h"
#include "TransitionChange.h"

namespace clang::ento::nvm {

template <typename Analyzer> class DataflowAnalysis {
  // df results
  using AbstractState = typename Analyzer::AbstractState;
  using FunctionResults = typename Analyzer::FunctionResults;
  using DataflowResults = typename Analyzer::DataflowResults;

  // context helpers
  using FunctionContext = std::pair<const FunctionDecl*, AbstractContext>;
  using FunctionContextSet = std::set<FunctionContext>;
  using FunctionContextMap = std::map<FunctionContext, FunctionContextSet>;

  // worklist
  template <typename WorkElement, int N>
  using Worklist = SmallVector<WorkElement, N>;
  using ContextWorklist = Worklist<FunctionContext, 100>;
  using BlockWorklist = Worklist<const AbstractBlock*, 200>;

  // data structures
  DataflowResults allResults;
  ContextWorklist contextWork;
  FunctionContextMap callers;
  FunctionContextSet active;

  // top info
  const FunctionDecl* topFunction;
  AnalysisManager* Mgr;
  Analyzer& analyzer;

  // interrupt data flow
  bool stopDataflow;

  void initTopEntryValues(const AbstractFunction* absFunction,
                          FunctionResults& results) {
    // initialize entry block
    auto* entryBlockKey = absFunction->getEntryBlockKey();
    AbstractState& state = results[entryBlockKey];

    // initialize all tracked variables for the entry block
    analyzer.initLatticeValues(state);
  }

  void initCalleeEntryValues(const AbstractFunction* absFunction,
                             FunctionResults& results) {
    auto* entryBlockKey = absFunction->getEntryBlockKey();
    AbstractState& state = results[entryBlockKey];

    // initialize from the function entry state
    auto* functionEntryKey = absFunction->getFunctionEntryKey();
    state = results[functionEntryKey];
  }

  FunctionResults& getFunctionResults(const AbstractFunction* absFunction,
                                      const AbstractContext& context) {
    FunctionResults& results = allResults[context];
    auto* functionEntryKey = absFunction->getFunctionEntryKey();

    if (!results.count(functionEntryKey)) {
      initTopEntryValues(absFunction, results);
    } else {
      initCalleeEntryValues(absFunction, results);
    }

    return results;
  }

  void addBlocksToWorklist(const AbstractFunction* absFunction,
                           BlockWorklist& blockWorkList) {
    for (auto& absBlock : absFunction->getReverseBlocks()) {
      blockWorkList.push_back(absBlock);
    }
  }

  AbstractState mergePrevStates(const AbstractBlock* absBlock,
                                FunctionResults& results) {
    auto* blockEntryKey = absBlock->getBlockEntryKey();

    AbstractState mergedState;
    AbstractState& inState = results[blockEntryKey];

    // start with current entry
    mergeInState(mergedState, inState);

    // get all prev blocks
    for (auto* predBlock : absBlock->getPredecessors()) {
      auto* blockExitKey = predBlock->getBlockExitKey();

      if (results.count(blockExitKey)) {
        AbstractState& predecessorState = results[blockExitKey];
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

  bool analyzeCall(const CallExpr* CE, AbstractState& callerState,
                   const AbstractFunction* absCaller,
                   const AbstractContext& context) {
    if (stopDataflow) {
      return false;
    }
    AbstractContext newContext(context, CE);
    const FunctionDecl* caller = absCaller->getFunction();
    const FunctionDecl* callee = CE->getDirectCallee();
    if (!callee)
      return false;

    const AbstractFunction* absCallee = analyzer.getAbstractFunction(callee);

    // prepare function contexts for caller and callee
    FunctionContext toCall = std::pair(callee, newContext);
    FunctionContext toUpdate = std::pair(caller, context);

    // get keys
    auto* calleeEntryKey = absCallee->getFunctionEntryKey();
    auto* calleeExitKey = absCallee->getFunctionExitKey();

    // get previously computed states in the context
    auto& calleeResults = allResults[newContext];
    AbstractState& calleeEntryState = calleeResults[calleeEntryKey];

    if (active.count(toCall) || callerState == calleeEntryState) {
      return false;
    }

    // update function entry
    calleeEntryState = callerState;

    // do dataflow inter-procedurally
    computeDataflow(callee, newContext);

    // get exit state
    AbstractState& calleeExitState = calleeResults[calleeExitKey];

    // if same result do not update
    if (callerState == calleeExitState) {
      return false;
    }

    // update caller state
    callerState = calleeExitState;
    callers[toCall].insert(toUpdate);

    return true;
  }

  bool isStateChange(TransitionChange change) {
    switch (change) {
    case TransitionChange::StateChange: {
      return true;
    } break;
    case TransitionChange::BugChange: {
      stopDataflow = true;
      contextWork.clear();
      return true;
    } break;
    default:
      return false;
    }
  }

  bool applyTransfer(const Stmt* S, AbstractState& state) {
    auto change = analyzer.handleStmt(S, state);
    return isStateChange(change);
  }

  bool analyzeStmt(const Stmt* S, AbstractState& state,
                   const AbstractFunction* absCaller,
                   const AbstractContext& context) {
    if (const CallExpr* CE = analyzer.getIpaCall(S)) {
      return analyzeCall(CE, state, absCaller, context);
    } else {
      return applyTransfer(S, state);
    }
  }

  void analyzeStmts(const AbstractBlock* absBlock, AbstractState& state,
                    FunctionResults& results, const AbstractFunction* absCaller,
                    const AbstractContext& context) {

    for (auto& absStmt : absBlock->getStmts()) {
      assert(absStmt);
      const Stmt* S = absStmt->getStmt();
      printStmt(S, *Mgr, "dd");
      if (analyzeStmt(S, state, absCaller, context)) {
        auto* stmtKey = absStmt->getStmtKey();
        results[stmtKey] = state;
      }
    }
  }

  void computeDataflow(const FunctionDecl* function, AbstractContext& context) {
    if (stopDataflow) {
      return;
    }

    const AbstractFunction* absFunction =
        analyzer.getAbstractFunction(function);

    active.insert({function, context});

    // initialize results
    FunctionResults& results = getFunctionResults(absFunction, context);

    // initialize worklist
    BlockWorklist blockWorklist;
    addBlocksToWorklist(absFunction, blockWorklist);

    while (!blockWorklist.empty()) {
      if (stopDataflow) {
        return;
      }

      const AbstractBlock* absBlock = blockWorklist.pop_back_val();

      auto* blockEntryKey = absBlock->getBlockEntryKey();
      auto* blockExitKey = absBlock->getBlockExitKey();

      // get previously computed states
      AbstractState& oldEntryState = results[blockEntryKey];
      AbstractState oldExitState = results[blockExitKey];

      // get current state
      AbstractState state = mergePrevStates(absBlock, results);

      // skip block if same result
      if (state == oldEntryState && !state.empty()) {
        continue;
      }

      // update entry based on predecessors
      results[blockEntryKey] = state;

      // todo go over statements
      analyzeStmts(absBlock, state, results, absFunction, context);

      // skip if state not updated
      if (state == oldExitState) {
        continue;
      }

      // update final state
      results[blockExitKey] = state;

      // do data flow for successive blocks
      for (auto* succBlock : absBlock->getSuccessors()) {
        blockWorklist.push_back(succBlock);
      }
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

    active.erase({function, context});
  }

public:
  DataflowAnalysis(const FunctionDecl* function, Analyzer& analyzer_)
      : topFunction(function), analyzer(analyzer_) {
    Mgr = analyzer.getMgr();
    contextWork.push_back({function, AbstractContext()});
    stopDataflow = false;
  }

  DataflowResults* computeDataflow() {
    while (!contextWork.empty()) {
      auto [function, context] = contextWork.pop_back_val();
      computeDataflow(function, context);
    }
    return &allResults;
  }
};

} // namespace clang::ento::nvm