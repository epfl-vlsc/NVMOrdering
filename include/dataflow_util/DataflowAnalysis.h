#pragma once
#include "Common.h"
#include "DataflowPrint.h"
#include "ProgramLocation.h"

namespace clang::ento::nvm {

template <typename Analyzer> class DataflowAnalysis {
  // df results
  using AbstractState = typename Analyzer::AbstractState;
  using FunctionResults = typename Analyzer::FunctionResults;
  using DataflowResults = typename Analyzer::DataflowResults;

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
  DataflowResults allResults;
  ContextWorklist contextWork;
  FunctionContextMap callers;
  FunctionContextSet active;

  // top info
  const FunctionDecl* topFunction;
  AnalysisManager* mgr;
  Analyzer analyzer;

  void initTopEntryValues(const FunctionDecl* function,
                          FunctionResults& results) {
    // initialize entry block
    const CFG* cfg = mgr->getCFG(function);
    ProgramLocation entryBlock = Forward::getEntryBlock(cfg);
    AbstractState& state = results[entryBlock];

    // initialize all tracked variables for the entry block
    analyzer.initLatticeValues(state);
  }

  void initCalleeEntryValues(const FunctionDecl* function,
                             FunctionResults& results) {
    const CFG* cfg = mgr->getCFG(function);
    ProgramLocation entryBlock = Forward::getEntryBlock(cfg);
    AbstractState& state = results[entryBlock];

    // initialize from the function entry state
    ProgramLocation functionEntryKey = Forward::getFunctionEntryKey(function);
    state = results[functionEntryKey];
  }

  FunctionResults& getFunctionResults(const FunctionDecl* function,
                                      PlContext& context) {
    FunctionResults& results = allResults[context];
    ProgramLocation functionEntryKey = Forward::getFunctionEntryKey(function);

    if (!results.count(functionEntryKey)) {
      printMsg("tol");
      initTopEntryValues(function, results);
    } else {
      printMsg("xol");
      initCalleeEntryValues(function, results);
    }

    return results;
  }

  void addBlocksToWorklist(BlockWorklist& blockWorkList,
                           const FunctionDecl* function) {
    const CFG* cfg = mgr->getCFG(function);
    for (const CFGBlock* block : Forward::getBlocks(cfg)) {
      blockWorkList.push_back(block);
    }
  }

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

  bool analyzeCall(const CallExpr* CE, AbstractState& callerState,
                   const FunctionDecl* caller, const PlContext& context) {
    PlContext newContext(context, CE);
    const FunctionDecl* callee = CE->getDirectCallee();
    if (!callee)
      return false;

    // prepare function contexts for caller and callee
    FunctionContext toCall = std::pair(callee, newContext);
    FunctionContext toUpdate = std::pair(caller, context);

    // get keys
    ProgramLocation calleeEntryKey = Forward::getFunctionEntryKey(callee);
    ProgramLocation calleeExitKey = Forward::getFunctionExitKey(callee, mgr);

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

  bool applyTransfer(const Stmt* S, AbstractState& state) {
    return analyzer.handleStmt(S, state);
  }

  bool analyzeStmt(const Stmt* S, AbstractState& state,
                   const FunctionDecl* caller, const PlContext& context) {
    if (const CallExpr* CE = analyzer.getIpaCall(S)) {
      return analyzeCall(CE, state, caller, context);
    } else {
      return applyTransfer(S, state);
    }
  }

  void analyzeStmts(const CFGBlock* block, AbstractState& state,
                    FunctionResults& results, const FunctionDecl* caller,
                    const PlContext& context) {

    for (const CFGElement element : Forward::getElements(block)) {
      if (Optional<CFGStmt> CS = element.getAs<CFGStmt>()) {
        const Stmt* S = CS->getStmt();
        assert(S);

        if (analyzeStmt(S, state, caller, context)) {
          ProgramLocation stmtKey = Forward::getStmtKey(S);
          results[stmtKey] = state;
        }
      }
    }
  }

  void computeDataflow(const FunctionDecl* function, PlContext& context) {
    active.insert({function, context});

    // initialize results
    FunctionResults& results = getFunctionResults(function, context);

    // initialize worklist
    BlockWorklist blockWorklist;
    addBlocksToWorklist(blockWorklist, function);

    while (!blockWorklist.empty()) {
      const CFGBlock* block = blockWorklist.pop_back_val();
      ProgramLocation entryKey = Forward::getEntryKey(block);
      ProgramLocation exitKey = Forward::getExitKey(block);

      // get previously computed states
      AbstractState& oldEntryState = results[entryKey];
      AbstractState oldExitState = results[exitKey];

      // get current state
      AbstractState state = mergePrevStates(entryKey, results);

      // skip block if same result
      if (state == oldEntryState && !state.empty()) {
        continue;
      }

      // update entry based on predecessors
      results[entryKey] = state;

      // todo go over statements
      analyzeStmts(block, state, results, function, context);

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

    active.erase({function, context});
  }

public:
  DataflowAnalysis(const FunctionDecl* function, Analyzer& analyzer_)
      : topFunction(function), analyzer(analyzer_) {
    mgr = analyzer.getMgr();
    contextWork.push_back({function, PlContext()});
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