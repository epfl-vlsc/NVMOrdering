#pragma once
#include "Common.h"
#include "reporting/BaseReporter.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

const std::string ReadError = "NVM Read Error";

class CheckBugs {
public:
  BugPtr CheckNotRead;

protected:
  CheckBugs(const CheckerBase& CB) {
    CheckNotRead.reset(new BugType(&CB, "Check is not read", ReadError));
  }
};

class ReadBugReporter : public BaseReporter, public CheckBugs {

public:
  ReadBugReporter(const CheckerBase& CB) : CheckBugs(CB) {}
};

} // namespace clang::ento::nvm