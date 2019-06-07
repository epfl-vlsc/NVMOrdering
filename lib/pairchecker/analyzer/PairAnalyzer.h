#pragma once
#include "../preprocess/PairParser.h"
#include "Common.h"
#include "LatticeValue.h"
#include "PairTransitions.h"
#include "dataflow_util/MainAnalyzer.h"
#include "dataflow_util/ProgramLocation.h"

namespace clang::ento::nvm {

class PairAnalyzer
    : public MainAnalyzer<PairVariables, PairFunctions, LatticeValue,
                          PairTransitions<PairVariables, PairFunctions>,
                          PairAnalyzer> {

public:
  PairAnalyzer& getAnalyzer() { return *this; };
};

} // namespace clang::ento::nvm