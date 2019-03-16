#pragma once
#include "Common.h"
#include "States.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

class WrongModelWalker : public BugReporterVisitorImpl<WrongModelWalker> {
  const Decl* D;

public:
  WrongModelWalker(const Decl* decl) : D(decl) {}
  void Profile(llvm::FoldingSetNodeID& ID) const {}

  std::shared_ptr<PathDiagnosticPiece> VisitNode(const ExplodedNode* N,
                                                 const ExplodedNode* PrevN,
                                                 BugReporterContext& BRC,
                                                 BugReport& BR);
};

class OrderingBugReporter {
  const std::string OrderingError = "NVM Ordering Error";

  // path-sensitive bug types
  std::unique_ptr<BugType> WrongWriteBugType;
  std::unique_ptr<BugType> WrongModelBugType;

public:
  OrderingBugReporter(const CheckerBase& CB) {
    WrongWriteBugType.reset(new BugType(&CB, "Wrong write", OrderingError));
    WrongModelBugType.reset(new BugType(&CB, "Wrong model", OrderingError));
  }

  void reportWriteBug(CheckerContext& C, const DeclaratorDecl* D,
                      const ExplodedNode* const ExplNode,
                      BugReporter& BReporter) const;
  void reportModelBug(CheckerContext& C, BugReporter& BReporter) const;
};

} // namespace clang::ento::nvm