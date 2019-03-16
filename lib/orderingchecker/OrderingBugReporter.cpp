#include "OrderingBugReporter.h"

namespace clang::ento::nvm {

std::shared_ptr<PathDiagnosticPiece>
WrongModelWalker::VisitNode(const ExplodedNode* N, const ExplodedNode* PrevN,
                            BugReporterContext& BRC, BugReport& BR) {

  llvm::outs() << "visit node\n";
  return nullptr;
}

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
                                         BugReporter& BReporter) const {

  ProgramStateRef State = C.getState();
  ExplodedNode* ErrNode = C.generateNonFatalErrorNode();
  bool isBuggy = false;

  llvm::outs() << ErrNode << "\n";

  for (auto& [dataDeclDecl, dclState] : State->get<DclMap>()) {
    ErrNode = C.generateNonFatalErrorNode();
  }

  /*
    // iterate over dcl
    for (auto& [dataDeclDecl, dclState] : State->get<DclMap>()) {

      llvm::outs() << dataDeclDecl << " dcl " << dataDeclDecl->getName() << " "
                   << dclState.getStateName() << "\n";
      if (!dclState.isPFenceCheck()) {
        isBuggy = true;
        if(ErrNode)

        ExplodedNode* ErrNode = C.generateNonFatalErrorNode();
        if (!ErrNode) {
          return;
        }
        BReporter.reportModelBug(C, ErrNode, C.getBugReporter());
      }
    }

    // iterate over scl
    for (auto& [dataDeclDecl, sclState] : State->get<SclMap>()) {
      llvm::outs() << dataDeclDecl << " scl " << dataDeclDecl->getName() << " "
                   << sclState.getStateName() << "\n";
      if (!sclState.isWriteCheck()) {
        ExplodedNode* ErrNode = C.generateNonFatalErrorNode();
        if (!ErrNode) {
          return;
        }

        BReporter.reportModelBug(C, ErrNode, C.getBugReporter());
      }
    }

    const LocationContext* LC = C.getLocationContext();
    const FunctionDecl* D = getFuncDecl(LC);
    std::string ErrorText =
        "Wrong model usage at function: " + D->getNameAsString();

    auto Report =
        llvm::make_unique<BugReport>(*WrongModelBugType, ErrorText, ExplNode);
    Report->markInteresting(LC);
    Report->addVisitor(llvm::make_unique<WrongModelWalker>());
    BReporter.emitReport(std::move(Report));
    */
}

} // namespace clang::ento::nvm