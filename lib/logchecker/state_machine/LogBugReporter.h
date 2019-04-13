#pragma once
#include "Common.h"
#include "reporting/BaseReporter.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

const std::string LogError = "NVM Log Error";

class LogBugs {
public:
  BugPtr WriteWithoutLogging;
  BugPtr AlreadyWritten;
  BugPtr AlreadyLogged;

protected:
  LogBugs(const CheckerBase& CB) {
    WriteWithoutLogging.reset(
        new BugType(&CB, "Writing without logging", LogError));
    AlreadyWritten.reset(new BugType(&CB, "Already written", LogError));
    AlreadyLogged.reset(new BugType(&CB, "Already logged", LogError));
  }
};

class LogBugReporter : public BaseReporter, public LogBugs {

public:
  LogBugReporter(const CheckerBase& CB) : LogBugs(CB) {}
};

} // namespace clang::ento::nvm