#pragma once
#include "Common.h"
#include "reporting/BaseReporter.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

const std::string LogError = "NVM Log Error";

class LogBugs {
public:
  BugPtr DoubleLog;
  BugPtr NotLogBeforeWrite;

protected:
  LogBugs(const CheckerBase& CB) {
    DoubleLog.reset(new BugType(&CB, "Already logged", LogError));
    NotLogBeforeWrite.reset(new BugType(&CB, "Not logged", LogError));
  }
};

class TxBugs {
public:
  BugPtr AccessOutsideTx;

protected:
  TxBugs(const CheckerBase& CB) {
    AccessOutsideTx.reset(new BugType(&CB, "Access outside tx", LogError));
  }
};

class LogBugReporter : public BaseReporter, public LogBugs, public TxBugs {

public:
  LogBugReporter(const CheckerBase& CB) : LogBugs(CB), TxBugs(CB) {}
};

} // namespace clang::ento::nvm