#pragma once
#include "Common.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

class OrderingBugReporter {
  const std::string OrderingError = "NVM Ordering Error";

  // path-sensitive bug types
  std::unique_ptr<BugType> WrongWriteDataBugType;
  std::unique_ptr<BugType> WrongWriteCheckBugType;
  std::unique_ptr<BugType> WrongFlushDataBugType;
  std::unique_ptr<BugType> WrongFlushCheckBugType;
  std::unique_ptr<BugType> WrongFenceBugType;
  std::unique_ptr<BugType> WrongModelBugType;

public:
  OrderingBugReporter(const CheckerBase& CB) {
    WrongWriteDataBugType.reset(
        new BugType(&CB, "Wrong write data", OrderingError));
    WrongWriteCheckBugType.reset(
        new BugType(&CB, "Wrong write check", OrderingError));
    WrongFlushDataBugType.reset(
        new BugType(&CB, "Wrong flush data", OrderingError));
    WrongFlushCheckBugType.reset(
        new BugType(&CB, "Wrong flush check", OrderingError));
    WrongFenceBugType.reset(
        new BugType(&CB, "Wrong fence check", OrderingError));
    WrongModelBugType.reset(new BugType(&CB, "Wrong model", OrderingError));
  }

  void reportFenceBug(const CallEvent& Call, CheckerContext& C,
                      const ExplodedNode* const ExplNode,
                      BugReporter& BReporter) const;
  void reportFlushDataBug(CheckerContext& C, const DeclaratorDecl* D,
                          const ExplodedNode* const ExplNode,
                          BugReporter& BReporter) const;
  void reportFlushCheckBug(CheckerContext& C, const DeclaratorDecl* D,
                           const ExplodedNode* const ExplNode,
                           BugReporter& BReporter) const;
  void reportWriteDataBug(CheckerContext& C, const DeclaratorDecl* D,
                          const ExplodedNode* const ExplNode,
                          BugReporter& BReporter) const;
  void reportWriteCheckBug(CheckerContext& C, const DeclaratorDecl* D,
                           const ExplodedNode* const ExplNode,
                           BugReporter& BReporter) const;
  void reportModelBug(CheckerContext& C, const ExplodedNode* const ExplNode,
                      BugReporter& BReporter) const;
};

} // namespace clang::ento::nvm