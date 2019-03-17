#pragma once
#include "Common.h"
#include "States.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

class RecoveryBugReporter {
  const std::string RecoveryError = "NVM Recovery Error";

  // path-sensitive bug types
  std::unique_ptr<BugType> WrongReadBugType;

public:
  RecoveryBugReporter(const CheckerBase& CB) {
    WrongReadBugType.reset(new BugType(&CB, "Wrong read", RecoveryError));
  }

  void reportReadBug(SVal Loc, CheckerContext& C, const DeclaratorDecl* D,
                      const ExplodedNode* const ExplNode,
                      BugReporter& BReporter) const {

    const FunctionDecl* FD = getTopFunction(C);

    std::string sbuf;
    llvm::raw_string_ostream ErrorOs(sbuf);
    ErrorOs << "Wrong read usage at " + FD->getName();

    auto Report = llvm::make_unique<BugReport>(*WrongReadBugType,
                                               ErrorOs.str(), ExplNode);
    Report->markInteresting(Loc);
    BReporter.emitReport(std::move(Report));
  }
};

} // namespace clang::ento::nvm