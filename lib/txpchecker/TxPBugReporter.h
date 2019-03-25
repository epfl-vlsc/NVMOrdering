#pragma once
#include "Common.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

class TxPBugReporter {
  const std::string TxPError = "NVM Transaction PMDK Error";

  // path-sensitive bug types
  std::unique_ptr<BugType> OutTxWriteBugType;

public:
  TxPBugReporter(const CheckerBase& CB) {
    OutTxWriteBugType.reset(
        new BugType(&CB, "Wrong write outside transaction", TxPError));
  }

  void reportOutTxWriteBug(const MemRegion* Region, CheckerContext& C,
                           const ExplodedNode* const ExplNode,
                           BugReporter& BReporter) const {
    const FunctionDecl* FD = getTopFunction(C);
    
    std::string sbuf;
    llvm::raw_string_ostream ErrorOs(sbuf);
    ErrorOs << "Access outside transaction at function: " + FD->getName();

    auto Report = llvm::make_unique<BugReport>(*OutTxWriteBugType,
                                               ErrorOs.str(), ExplNode);
    Report->markInteresting(Region);
    BReporter.emitReport(std::move(Report));
  }
};

} // namespace clang::ento::nvm