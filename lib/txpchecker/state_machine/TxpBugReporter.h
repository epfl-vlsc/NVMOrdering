#pragma once
#include "reporting/BaseReporter.h"
#include "Common.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

class TxpBugReporter : public BaseReporter {
  const std::string TxPError = "NVM Transaction PMDK Error";

public:
  // path-sensitive bug types
  BugPtr AccessOutTxBugType;
  BugPtr DoubleLogBugType;
  BugPtr NotLogBeforeWriteBugType;
  BugPtr NotTxPairBugType;

public:
  TxpBugReporter(const CheckerBase& CB) {
    AccessOutTxBugType.reset(
        new BugType(&CB, "Access outside transaction", TxPError));
    DoubleLogBugType.reset(new BugType(&CB, "Double logging", TxPError));
    NotLogBeforeWriteBugType.reset(
        new BugType(&CB, "Not logged before write", TxPError));
    NotTxPairBugType.reset(
        new BugType(&CB, "More tx end than tx beg", TxPError));
  }
};

} // namespace clang::ento::nvm