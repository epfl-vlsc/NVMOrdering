#pragma once
#include "Common.h"

namespace clang::ento::nvm {

class DataFlow {

public:
  DataFlow() {}

  void applyTransfer(const Stmt* S, AbstractState& state) {}

  void mergePrevStates(const CFGBlock* block, FunctionResults& results) {}

  void analyzeCall(const CallExpr* CE, AbstractState& state,
                   const FunctionContext& context) {}

  void computeDataFlow(const CallExpr* CE, const FunctionContext& context) {}

  AllResults analyze(){}
};

} // namespace clang::ento::nvm