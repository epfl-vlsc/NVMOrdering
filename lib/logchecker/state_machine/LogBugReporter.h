#pragma once
#include "Common.h"
#include "reporting/BaseReporter.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

const std::string LogError = "NVM Log Error";

class LogBugs {
public:
  BugPtr DoubleLogBugType;
  BugPtr NotLogBeforeWriteBugType;

protected:
  LogBugs(const CheckerBase& CB) {
    DoubleLogBugType.reset(
        new BugType(&CB, "Already logged", LogError));
    NotLogBeforeWriteBugType.reset(new BugType(&CB, "Not logged", LogError));
  }
};

class LogBugReporter : public BaseReporter, public LogBugs {

public:
  LogBugReporter(const CheckerBase& CB) : LogBugs(CB) {}
};

} // namespace clang::ento::nvm