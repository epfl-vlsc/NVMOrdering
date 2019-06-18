#pragma once

#include "../preprocess/LogFunctions.h"
#include "../preprocess/LogGlobalParser.h"
#include "../preprocess/LogVariables.h"
#include "Common.h"
#include "LogStmtParser.h"
#include "LogTransitions.h"
#include "analysis_util/MainAnalyzer.h"
#include "dataflow_util/DataflowManager.h"
#include "global_util/GlobalInfo.h"

namespace clang::ento::nvm {

using Globals = GlobalInfo<LogVariables, LogFunctions>;
using StmtParser = LogStmtParser<Globals>;
using BReporter = LogBugReporter<Globals>;
using Transitions = LogTransitions<Globals, StmtParser, BReporter>;
using Manager = DataflowManager<Globals, StmtParser, BReporter, Transitions>;

using Parser = LogGlobalParser<Globals>;

class LogAnalyzer : public MainAnalyzer<Manager, Parser> {};

} // namespace clang::ento::nvm