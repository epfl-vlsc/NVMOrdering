#pragma once

#include "../preprocess/FlushedFunctions.h"
#include "../preprocess/FlushedGlobalParser.h"
#include "../preprocess/FlushedVariables.h"
#include "Common.h"
#include "FlushedStmtParser.h"
#include "FlushedTransitions.h"
#include "analysis_util/MainAnalyzer.h"
#include "dataflow_util/DataflowManager.h"
#include "global_util/GlobalInfo.h"

namespace clang::ento::nvm {

using Globals = GlobalInfo<FlushedVariables, FlushedFunctions>;
using StmtParser = FlushedStmtParser<Globals>;
using BReporter = FlushedBugReporter<Globals>;
using Transitions = FlushedTransitions<Globals, StmtParser, BReporter>;
using Manager = DataflowManager<Globals, StmtParser, BReporter, Transitions>;

using Parser = FlushedGlobalParser<Globals>;

class FlushedAnalyzer : public MainAnalyzer<Manager, Parser> {};

} // namespace clang::ento::nvm