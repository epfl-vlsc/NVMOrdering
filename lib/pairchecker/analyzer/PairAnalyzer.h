#pragma once
#include "../preprocess/PairParser.h"
#include "Common.h"
#include "LatticeValue.h"
#include "dataflow_util/MainAnalyzer.h"
#include "dataflow_util/ProgramLocation.h"

namespace clang::ento::nvm {

class PairAnalyzer
    : public MainAnalyzer<PairVariables, PairFunctions, LatticeValue, PairAnalyzer> {

public:
  void initLatticeValues(AbstractState& state) {
    for (auto usedVar : activeUnitInfo->getUsedVars()) {
      auto& [isDcl, isScl] = vars.getDsclInfo(usedVar);
      auto lv = LatticeValue::getInit(isDcl, isScl);
      state[usedVar] = lv;
    }
  }

  virtual PairAnalyzer& getAnalyzer(){
    return *this;
  };
};

} // namespace clang::ento::nvm