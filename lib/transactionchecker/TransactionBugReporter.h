#pragma once
#include "Common.h"
#include "States.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

class TransactionBugReporter {
  const std::string TransactionError = "NVM Transaction Error";

  // path-sensitive bug types
  std::unique_ptr<BugType> OutTxWriteBugType;

public:
  TransactionBugReporter(const CheckerBase& CB) {
    OutTxWriteBugType.reset(
        new BugType(&CB, "Wrong write outside transaction", TransactionError));
  }

  void reportOutTxWriteBug(SVal Loc, CheckerContext& C, const DeclaratorDecl* D,
                           const ExplodedNode* const ExplNode,
                           BugReporter& BReporter) const {
    /*
    const FunctionDecl* FD = getTopFunction(C);

    std::string sbuf;
    llvm::raw_string_ostream ErrorOs(sbuf);
    ErrorOs << "Wrong data usage at " + FD->getName();

    auto Report = llvm::make_unique<BugReport>(*OutTxWriteBugType, ErrorOs.str(),
                                               ExplNode);
    Report->markInteresting(Loc);
    BReporter.emitReport(std::move(Report));
    */
  }
};

} // namespace clang::ento::nvm