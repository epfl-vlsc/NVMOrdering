#pragma once

#include "../preprocess/PairFunctions.h"
#include "../preprocess/PairGlobalParser.h"
#include "../preprocess/PairVariables.h"
#include "Common.h"
#include "PairStmtParser.h"
#include "PairTransitions.h"
#include "analysis_util/MainAnalyzer.h"
#include "dataflow_util/DataflowManager.h"
#include "global_util/GlobalInfo.h"

namespace clang::ento::nvm {

using Globals = GlobalInfo<PairVariables, PairFunctions>;
using StmtParser = PairStmtParser<Globals>;
using BReporter = PairBugReporter<Globals>;
using Transitions = PairTransitions<Globals, StmtParser, BReporter>;
using Manager = DataflowManager<Globals, StmtParser, BReporter, Transitions>;

using Parser = PairGlobalParser<Globals>;

class PairAnalyzer : public MainAnalyzer<Manager, Parser> {};

} // namespace clang::ento::nvm