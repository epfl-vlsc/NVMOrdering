#pragma once
#include "Common.h"
#include "DfUtil.h"

namespace clang::ento::nvm {


class DataFlow {
  using ContextWorklist = std::vector<ContextFunction>;
  using ContextFunctionSet = std::set<ContextFunction>;
  using ContextFunctionMap = std::map<ContextFunction, ContextFunctionSet>;

  using BlockWorklist = std::vector<const CFGBlock*>;

  AllResults allResults;
  ContextWorklist contextWork;
  ContextFunctionMap callers;
  ContextFunctionSet active;

public:
  DataFlow() {}

  void applyTransfer(const Stmt* S, AbstractState& state) {}

  void mergePrevStates(const CFGBlock* block, FunctionResults& results) {}

  void analyzeCall(const CallExpr* CE, AbstractState& state,
                   const FunctionContext& context) {}

  void computeDataFlow(const CallExpr* CE, const FunctionContext& context) {}

  AllResults analyze() {
    
  }
};

} // namespace clang::ento::nvm