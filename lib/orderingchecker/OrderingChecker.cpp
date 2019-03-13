//===-- OrderingChecker.cpp -----------------------------------------*
#include "OrderingChecker.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerRegistry.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "llvm/Support/raw_ostream.h"
#include <utility>

using namespace clang;
using namespace ento;

namespace {

class OrderingChecker : public Checker<check::EndAnalysis, 
                                      check::PreCall,
                                      check::BeginFunction> {

public:
  OrderingChecker();

  void checkBeginFunction(CheckerContext &Ctx) const;

  void checkEndAnalysis(ExplodedGraph &G, BugReporter &BR,
                        ExprEngine &Eng) const;

  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
};

} // end anonymous namespace

OrderingChecker::OrderingChecker(){}

void OrderingChecker::checkBeginFunction(CheckerContext &C) const{
  const StackFrameContext* sfc = C.getStackFrame();
}

void OrderingChecker::checkPreCall(const CallEvent &Call, CheckerContext &C) const{
  //Call.dump();
}

void OrderingChecker::checkEndAnalysis(ExplodedGraph &G, BugReporter &BR,
                        ExprEngine &Eng) const {
  llvm::errs() << "End of analysis\n";
}

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(CheckerRegistry &registry) {
  registry.addChecker<OrderingChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
