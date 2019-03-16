#include "OrderingBugReporter.h"

namespace clang::ento::nvm {

void OrderingBugReporter::reportWriteBug(CheckerContext& C,
                                         const DeclaratorDecl* D,
                                         const ExplodedNode* const ExplNode,
                                         BugReporter& BReporter) const {
  std::string ErrorText = "Wrong write usage";
  auto Report =
      llvm::make_unique<BugReport>(*WrongWriteBugType, ErrorText, ExplNode);
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
  Report->addVisitor(llvm::make_unique<WrongModelWalker>());
  BReporter.emitReport(std::move(Report));
}

} // namespace clang::ento::nvm