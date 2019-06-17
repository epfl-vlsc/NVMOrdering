#pragma once
#include "Common.h"
#include "reporting/BaseReporter.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

const std::string MainError = "NVM Ptr Error";

class PtrBugs {
public:
  BugPtr NotFlushBug;

protected:
  PtrBugs(const CheckerBase& CB) {
    NotFlushBug.reset(new BugType(&CB, "not flush", MainError));
  }
};

class PtrBugReporter : public BaseReporter, public PtrBugs {

public:
  PtrBugReporter(const CheckerBase& CB) : PtrBugs(CB) {}
};

} // namespace clang::ento::nvm