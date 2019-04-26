//===-- ExpChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "ExpChecker.h"

namespace clang::ento::nvm {

void ExpChecker::checkASTDecl(const RecordDecl* RD, AnalysisManager& Mgr,
                              BugReporter& BR) const {
}

void ExpChecker::checkASTDecl(const TranslationUnitDecl* CTUD,
                              AnalysisManager& Mgr, BugReporter& BR) const {}

void ExpChecker::checkBeginFunction(CheckerContext& C) const {
  const LocationContext* LC = C.getLocationContext();
  const Decl* BD = LC->getDecl();
  if (const FunctionDecl* FD = dyn_cast_or_null<FunctionDecl>(BD)) {
    auto fncName = FD->getNameAsString();
    if (C.inTopFrame() && fncName == "insert") {
      ExplodedNode* ErrNode = C.generateErrorNode();
      if (!ErrNode)
        return;
    }
  }
}

void ExpChecker::checkEndFunction(CheckerContext& C) const {}

void ExpChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                           CheckerContext& C) const {

  unsigned kind = Loc.getRawKind();
  llvm::errs() << "bind:";
  Loc.dump();
  llvm::errs() << " " << kind;
  llvm::errs() << "\n";
}

void printArg(const CallEvent& Call, CheckerContext& C) {
  SVal Loc = Call.getArgSVal(0);
  unsigned kind = Loc.getRawKind();
  llvm::errs() << "arg:";
  Loc.dump();
  llvm::errs() << " " << kind;
  llvm::errs() << "\n";
}

void printReturn(const CallEvent& Call, CheckerContext& C) {
  SVal Loc = Call.getReturnValue();
  unsigned kind = Loc.getRawKind();
  llvm::errs() << "return:";
  Loc.dump();
  llvm::errs() << " " << kind;
  llvm::errs() << "\n";
}

void ExpChecker::checkPostCall(const CallEvent& Call, CheckerContext& C) const {
  const Decl* BD = Call.getDecl();
  if (const FunctionDecl* FD = dyn_cast_or_null<FunctionDecl>(BD)) {
    std::string fncName = FD->getNameAsString();
    llvm::errs() << "function:";
    llvm::errs() << fncName << "\n";
  }
}

void ExpChecker::checkPreCall(const CallEvent& Call, CheckerContext& C) const {}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::ExpChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
