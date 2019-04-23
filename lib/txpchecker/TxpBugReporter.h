#pragma once
#include "BaseReporter.h"
#include "Common.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

class TxPBugReporter : public BaseReporter {
  const std::string TxPError = "NVM Transaction PMDK Error";

public:
  // path-sensitive bug types
  BugPtr WriteOutTxBugType;
  BugPtr DoubleWriteBugType;
  BugPtr DoubleLogBugType;
  BugPtr NotLogBeforeWriteBugType;
  BugPtr NotTxPairBugType;

public:
  TxPBugReporter(const CheckerBase& CB) {
    WriteOutTxBugType.reset(
        new BugType(&CB, "Writing outside transaction", TxPError));
    DoubleWriteBugType.reset(
        new BugType(&CB, "Writing the same thing twice", TxPError));
    DoubleLogBugType.reset(new BugType(&CB, "Double logging", TxPError));
    NotLogBeforeWriteBugType.reset(
        new BugType(&CB, "Not logged before write", TxPError));
    NotTxPairBugType.reset(
        new BugType(&CB, "More tx end than tx beg", TxPError));
  }
};

} // namespace clang::ento::nvm