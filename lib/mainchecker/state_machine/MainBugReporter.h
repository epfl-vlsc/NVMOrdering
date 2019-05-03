#pragma once
#include "Common.h"
#include "reporting/BaseReporter.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

const std::string MainError = "NVM Main Error";

class MainBugs {
public:
  BugPtr CommitBug;
  BugPtr DoubleFlushBug;
  BugPtr FlushBug;

protected:
  MainBugs(const CheckerBase& CB) {
    CommitBug.reset(new BugType(&CB, "commit", MainError));
    DoubleFlushBug.reset(new BugType(&CB, "flush", MainError));
    FlushBug.reset(new BugType(&CB, "doubleflush", MainError));
  }
};

class MainBugReporter : public BaseReporter, public MainBugs {

public:
  MainBugReporter(const CheckerBase& CB) : MainBugs(CB) {}
};

} // namespace clang::ento::nvm