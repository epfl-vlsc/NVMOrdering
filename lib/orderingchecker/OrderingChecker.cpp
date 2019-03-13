//===-- OrderingChecker.cpp -----------------------------------------*
#include "OrderingChecker.h"

namespace clang::ento::nvm{

OrderingChecker::OrderingChecker(){}

void OrderingChecker::checkBeginFunction(CheckerContext &C) const{
  const StackFrameContext* sfc = C.getStackFrame();
  (void)sfc;
}

void OrderingChecker::checkPreCall(const CallEvent &Call, CheckerContext &C) const{
  //Call.dump();
}

void OrderingChecker::checkEndAnalysis(ExplodedGraph &G, BugReporter &BR,
                        ExprEngine &Eng) const {
  llvm::errs() << "End of analysis\n";
}

} //namespace nvm


extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry &registry) {
  registry.addChecker<clang::ento::nvm::OrderingChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
