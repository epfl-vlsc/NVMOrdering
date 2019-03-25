//===-- OrderingChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "MultiCompileChecker.h"

namespace clang::ento::nvm {

void MultiCompileChecker::checkPreCall(const CallEvent& Call,
                                       CheckerContext& C) const {  
  if(auto* II = Call.getCalleeIdentifier()){
    StringRef name = II->getName();
    //llvm::outs() << name << "\n";
  }
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::MultiCompileChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
