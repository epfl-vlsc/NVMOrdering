#pragma once
#include "AbstractProgram.h"
#include "Common.h"
#include "DataflowAnalysis.h"

namespace clang::ento::nvm {

template <typename Vars, typename Functions, typename LatVal,
          typename Transitions, typename Parser, typename SubClass>
class MainAnalyzer {
public:
  using FunctionInfo = typename Functions::FunctionInfo;
  using LatVar = const NamedDecl*;
  using AbstractState = std::map<LatVar, LatVal>;
  using FunctionResults = std::map<const AbstractLocation*, AbstractState>;
  using DataflowResults = std::map<AbstractContext, FunctionResults>;

protected:
  // program helpers
  AnalysisManager* Mgr;
  BugReporter* BR;
  const CheckerBase* CB;

  // whole program data structures
  Vars vars;
  Functions funcs;
  Transitions transitions;
  AbstractProgram abstractProgram;

  // convenient data structure per unit analysis
  FunctionInfo* activeUnitInfo;
  DataflowResults* dataflowResults;

  void parseTUD(TranslationUnitDecl* TUD) {
    ASTContext& astContext = Mgr->getASTContext();

    // parse entire translation unit
    Parser parser(vars, funcs, astContext);
    parser.TraverseDecl(TUD);
    parser.fillStructures();

    // initialize transitions
    transitions.initAll(vars, funcs, Mgr, BR, CB);
    vars.dump();
    funcs.dump();

    // create abstract graphs
    printMsg("parse");
    AbstractProgramBuilder programBuilder(abstractProgram, transitions);
    printMsg("parse2");
    abstractProgram.dump(Mgr);

    printMsg("abs program");
  }

  void doDataflowFD(const FunctionDecl* FD) {
    printND(FD, "***analyzing function***");

    // get unit info and current transitions
    activeUnitInfo = &funcs.getUnitInfo(FD);
    activeUnitInfo->dump();
    transitions.initUnit(activeUnitInfo);

    // run data flow analysis
    DataflowAnalysis dataflowAnalysis(FD, getAnalyzer());
    dataflowResults = dataflowAnalysis.computeDataflow();
    dumpDataflowResults(dataflowResults, Mgr);
  }

  void doDataflowTUD() {
    for (const FunctionDecl* FD : funcs) {
      // run data flow on a function inter-procedurally
      doDataflowFD(FD);
    }
  }

public:
  void analyzeTUD(TranslationUnitDecl* TUD, AnalysisManager& Mgr_,
                  BugReporter& BR_, const CheckerBase* CB_) {
    // init program helpers
    Mgr = &Mgr_;
    BR = &BR_;
    CB = CB_;

    // parse entire program
    parseTUD(TUD);

    // run data flow analysis on units
    doDataflowTUD();
  }

  AnalysisManager* getMgr() { return Mgr; }

  const CallExpr* getIpaCall(const Stmt* S) {
    if (const CallExpr* CE = dyn_cast<CallExpr>(S)) {
      const FunctionDecl* calleeFD = CE->getDirectCallee();
      if (!calleeFD)
        return nullptr;

      if (!funcs.isSkipFunction(calleeFD)) {
        return CE;
      }
    }

    return nullptr;
  }

  void initLatticeValues(AbstractState& state) {
    transitions.initLatticeValues(state);
  }

  auto handleStmt(const AbstractStmt* absStmt, AbstractState& state) {
    return transitions.handleStmt(absStmt, state);
  }

  const AbstractFunction* getAbstractFunction(const FunctionDecl* function) {
    return abstractProgram.getAbstractFunction(function);
  }

  virtual SubClass& getAnalyzer() = 0;
};

} // namespace clang::ento::nvm