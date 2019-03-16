#include "OrderingBugReporter.h"

namespace clang::ento::nvm {

void OrderingBugReporter::reportFenceBug(const CallEvent& Call,
                                         CheckerContext& C,
                                         const ExplodedNode* const ExplNode,
                                         BugReporter& BReporter) const {
  std::string ErrorText = "Wrong fence usage";
  auto Report =
      llvm::make_unique<BugReport>(*WrongFenceBugType, ErrorText, ExplNode);
  BReporter.emitReport(std::move(Report));
}
void OrderingBugReporter::reportFlushDataBug(CheckerContext& C,
                                             const DeclaratorDecl* D,
                                             const ExplodedNode* const ExplNode,
                                             BugReporter& BReporter) const {
  std::string ErrorText = "Wrong flush data usage";
  auto Report =
      llvm::make_unique<BugReport>(*WrongFlushDataBugType, ErrorText, ExplNode);
  BReporter.emitReport(std::move(Report));
}
void OrderingBugReporter::reportFlushCheckBug(
    CheckerContext& C, const DeclaratorDecl* D,
    const ExplodedNode* const ExplNode, BugReporter& BReporter) const {
  std::string ErrorText = "Wrong flush check usage";
  auto Report = llvm::make_unique<BugReport>(*WrongFlushCheckBugType, ErrorText,
                                             ExplNode);
  BReporter.emitReport(std::move(Report));
}
void OrderingBugReporter::reportWriteDataBug(CheckerContext& C,
                                             const DeclaratorDecl* D,
                                             const ExplodedNode* const ExplNode,
                                             BugReporter& BReporter) const {
  std::string ErrorText = "Wrong write data usage";
  auto Report =
      llvm::make_unique<BugReport>(*WrongWriteDataBugType, ErrorText, ExplNode);
  BReporter.emitReport(std::move(Report));
}
void OrderingBugReporter::reportWriteCheckBug(
    CheckerContext& C, const DeclaratorDecl* D,
    const ExplodedNode* const ExplNode, BugReporter& BReporter) const {
  std::string ErrorText = "Wrong write check usage";
  auto Report = llvm::make_unique<BugReport>(*WrongWriteCheckBugType, ErrorText,
                                             ExplNode);
  BReporter.emitReport(std::move(Report));
}

void OrderingBugReporter::reportModelBug(CheckerContext& C,
                                         const ExplodedNode* const ExplNode,
                                         BugReporter& BReporter) const {
  const LocationContext* LC = C.getLocationContext();
  const FunctionDecl* D = getFuncDecl(LC);
  std::string ErrorText =
      "Wrong model usage at function: " + D->getNameAsString();

  auto Report =
      llvm::make_unique<BugReport>(*WrongModelBugType, ErrorText, ExplNode);
  Report->markInteresting(LC);

  BReporter.emitReport(std::move(Report));
}

} // namespace clang::ento::nvm