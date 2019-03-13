#pragma once
#include "OrderingBugReporter.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerRegistry.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "llvm/Support/raw_ostream.h"
#include <utility>

constexpr const char * CHECKER_PLUGIN_NAME = "nvm.orderingchecker";

namespace clang::ento::nvm{

class OrderingChecker : public Checker<check::EndAnalysis, 
                                      check::PreCall,
                                      check::BeginFunction> {

public:
  OrderingChecker();

  void checkBeginFunction(CheckerContext &Ctx) const;

  void checkEndAnalysis(ExplodedGraph &G, BugReporter &BR,
                        ExprEngine &Eng) const;

  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;

private:
  OrderingBugReporter BReporter;
};

} // end anonymous namespace

