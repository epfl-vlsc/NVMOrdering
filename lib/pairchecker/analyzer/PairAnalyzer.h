#pragma once
#include "../preprocess/PairParser.h"
#include "Common.h"
#include "PairLattice.h"
#include "PairTransitions.h"
#include "dataflow_util/MainAnalyzer.h"

namespace clang::ento::nvm {

class PairAnalyzer
    : public MainAnalyzer<PairVariables, PairFunctions, PairLattice,
                          PairTransitions<PairVariables, PairFunctions>,
                          PairParser, PairAnalyzer> {

public:
  PairAnalyzer& getAnalyzer() { return *this; };
};

} // namespace clang::ento::nvm