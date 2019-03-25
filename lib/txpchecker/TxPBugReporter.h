#pragma once
#include "Common.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

class TxPBugReporter {
  const std::string TxPError = "NVM Transaction PMDK Error";

  // path-sensitive bug types
  std::unique_ptr<BugType> TxWriteBugType;
  std::unique_ptr<BugType> TxLogBugType;

public:
  TxPBugReporter(const CheckerBase& CB) {
    TxWriteBugType.reset(
        new BugType(&CB, "Writing outside transaction", TxPError));
    TxLogBugType.reset(new BugType(&CB, "Writing without logging", TxPError));
  }

  void reportTxWriteBug(const MemRegion* Region, CheckerContext& C,
                        const ExplodedNode* const ExplNode,
                        BugReporter& BReporter) const {
    const FunctionDecl* FD = getTopFunction(C);

    std::string sbuf;
    llvm::raw_string_ostream ErrorOs(sbuf);
    ErrorOs << "Access outside transaction at function: " + FD->getName();

    auto Report =
        llvm::make_unique<BugReport>(*TxWriteBugType, ErrorOs.str(), ExplNode);
    Report->markInteresting(Region);
    BReporter.emitReport(std::move(Report));
  }

  void reportTxLogBug(const MemRegion* Region, CheckerContext& C,
                      const ExplodedNode* const ExplNode,
                      BugReporter& BReporter) const {
    const FunctionDecl* FD = getTopFunction(C);

    std::string sbuf;
    llvm::raw_string_ostream ErrorOs(sbuf);
    ErrorOs << "Writing without logging: " + FD->getName();

    auto Report =
        llvm::make_unique<BugReport>(*TxLogBugType, ErrorOs.str(), ExplNode);
    Report->markInteresting(Region);
    BReporter.emitReport(std::move(Report));
  }
};

} // namespace clang::ento::nvm