#pragma once
#include "Common.h"
#include "DfUtil.h"
#include "DsclState.h"
#include "ProgramLocation.h"

namespace clang::ento::nvm {

class DataFlow {
  // result types
  using AbstractState = std::map<ProgramLocation, DsclValue>;
  using FunctionResults = std::map<ProgramLocation, AbstractState>;
  using ContextResults = std::map<const CFG*, FunctionResults>;
  using AllResults = std::map<PlContext, ContextResults>;

  // context helpers
  using ContextFunction = std::pair<PlContext, const CFG*>;
  using ContextFunctionSet = std::set<ContextFunction>;
  using ContextFunctionMap = std::map<ContextFunction, ContextFunctionSet>;

  // worklist
  template <typename WorkElement> using Worklist = SmallVector<WorkElement, 20>;
  using ContextWorklist = Worklist<ContextFunction>;
  using BlockWorklist = Worklist<const CFGBlock*>;

  // data structures
  AllResults allResults;
  ContextWorklist contextWork;
  ContextFunctionMap callers;
  ContextFunctionSet active;

  FunctionResults& initFunctionResults(const CFG* function,
                                       PlContext& context) {
    ContextResults& contextResults = allResults[context];
    FunctionResults& functionResults = contextResults[function];

    llvm::instructions

    return functionResults;
  }

public:
  DataFlow(const CFG* function) {
    contextWork.emplace_back(std::pair(PlContext{}, function));
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