//===-- LogChecker.cpp -----------------------------------------*

#include "LogChecker.h"

namespace clang::ento::nvm {

void LogChecker::checkASTDecl(const FunctionDecl* FD, AnalysisManager& Mgr,
                              BugReporter& BR) const {
  logFncs.insertIfKnown(FD);
}

void LogChecker::checkASTDecl(const FieldDecl* FD, AnalysisManager& Mgr,
                              BugReporter& BR) const {}

void LogChecker::checkBeginFunction(CheckerContext& C) const {}

void LogChecker::checkPostCall(const CallEvent& Call, CheckerContext& C) const {
  const FunctionDecl* FD = getFuncDecl(Call);
  if(!FD){
    return;
  }

  if(logFncs.isLogFunction(FD)){
    handleLog(Call, C);
  }
}

void LogChecker::handleLog(const CallEvent& Call, CheckerContext& C) const{
  const FunctionDecl* FD = getFuncDecl(Call);
  llvm::errs() << FD->getNameAsString() << "\n";

  SVal Loc = Call.getArgSVal(0);
  printLoc(Loc, "log:");
}

void LogChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                           CheckerContext& C) const {}

void LogChecker::checkBranchCondition(const Stmt* S, CheckerContext& C) const {}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::LogChecker>(CHECKER_PLUGIN_NAME,
                                                    "Checks logging use");
}
