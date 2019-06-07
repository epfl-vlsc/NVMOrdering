#pragma once
#include "../preprocess/PairParser.h"
#include "Common.h"
#include "LatticeValue.h"
#include "dataflow_util/ProgramLocation.h"

namespace clang::ento::nvm {

class PairAnalyzer {
  using FunctionInfo = typename PairFunctions::FunctionInfo;
  using LatVar = const NamedDecl*;
  using LatVal = LatticeValue;
  using AbstractState = std::map<LatVar, LatVal>;
  using FunctionResults = std::map<ProgramLocation, AbstractState>;
  using DataflowResults = std::map<PlContext, FunctionResults>;

  // program helpers
  AnalysisManager* Mgr;
  BugReporter* BR;

  // whole program data structures
  PairVariables pairVars;
  PairFunctions pairFuncs;

  // convenient data structure per unit analysis
  FunctionInfo* activeUnitInfo;
  DataflowResults* dataflowResults;

  void parseTUD(TranslationUnitDecl* TUD) {
    ASTContext& astContext = Mgr->getASTContext();
    PairParser pairParser(pairVars, pairFuncs, astContext);
    pairParser.TraverseDecl(TUD);
    pairParser.fillStructures();
    // pairParser.createGraphs();

    pairFuncs.dump();
    pairVars.dump();
  }

  void doDataflowFD(const FunctionDecl* FD){
    printND(FD, "***analyzing function***");

    //get unit info
    activeUnitInfo = &pairFuncs.getUnitInfo(FD);
    activeUnitInfo->dump();

    //DataFlow dataflow(FD, *this, mgr, BR);
  }

  void doDataflowTUD(){
    for(const FunctionDecl* FD: pairFuncs){
      doDataflowFD(FD);
    }
  }

public:
  
  void analyzeTUD(TranslationUnitDecl* TUD, AnalysisManager& Mgr_,
                  BugReporter& BR_) {
    // init program helpers
    Mgr = &Mgr_;
    BR = &BR_;

    // parse entire program
    parseTUD(TUD);
    

    //run data flow analysis on units
    doDataflowTUD();

    /*
     // run data flow analysis and report errors
     const FunctionDecl* FD = dyn_cast<FunctionDecl>(D);
     if (!FD || !analyzer.isAnalyzedFunction(FD)) {
       return;
     }

     // analyze single function inter-procedurally
     DataFlow dataflow(FD, lattice, mgr, BR);
     dataflow.computeDataFlow();
     dataflow.dump();
     dataflow.reportBugs();
     */
  }

  

  PairVariables& getMainVars() { return pairVars; }

  PairFunctions& getMainFncs() { return pairFuncs; }

  void initLatticeValues(AbstractState& state) {
    for (auto usedVar : activeUnitInfo->getUsedVars()) {
      auto& [isDcl, isScl] = pairVars.getDsclInfo(usedVar);
      auto lv = LatticeValue::getInit(isDcl, isScl);
      state[usedVar] = lv;
    }
  }

  bool isAnalyzedFunction(const FunctionDecl* FD) const {
    return pairFuncs.isAnalyzedFunction(FD);
  }

  bool isIpaCall(const Stmt* S) {
    if (const CallExpr* CE = dyn_cast<CallExpr>(S)) {
      const FunctionDecl* calleeFD = CE->getDirectCallee();
      if (!calleeFD)
        return false;

      return !pairFuncs.isSkipFunction(calleeFD);
    }

    return false;
  }

  bool handleWrite(const BinaryOperator* BO, AbstractState& state) {
    Expr* LHS = BO->getLHS();
    if (const MemberExpr* ME = dyn_cast<MemberExpr>(LHS)) {
      const ValueDecl* VD = ME->getMemberDecl();
      if (pairVars.isUsedVar(VD)) {
        state[VD] = LatticeValue::getWrite(state[VD]);
        return true;
      }
    }

    return false;
  }

  bool handleFence(const CallExpr* CE, AbstractState& state, bool isPfence) {
    llvm::report_fatal_error("not implemented");
    return false;
  }

  bool handleFlush(const CallExpr* CE, AbstractState& state, bool isPfence) {
    const Expr* arg0 = CE->getArg(0);
    if (const ImplicitCastExpr* ICE = dyn_cast<ImplicitCastExpr>(arg0)) {
      const Stmt* Child1 = getNthChild(ICE, 0);
      if (const UnaryOperator* UO = dyn_cast<UnaryOperator>(Child1)) {
        const Stmt* expr = UO->getSubExpr();
        if (const MemberExpr* ME = dyn_cast<MemberExpr>(expr)) {
          const ValueDecl* VD = ME->getMemberDecl();
          if (!pairVars.isUsedVar(VD)) {
            return false;
          }

          if (isPfence) {
            state[VD] = LatticeValue::getPfence(state[VD]);
          } else {
            state[VD] = LatticeValue::getFlush(state[VD]);
          }
          return true;
        }
      }
    }
    return false;
  }

  bool handleCall(const CallExpr* CE, AbstractState& state) {
    const FunctionDecl* FD = CE->getDirectCallee();
    if (!FD)
      return false;

    if (pairFuncs.isFlushFenceFunction(FD)) {
      return handleFlush(CE, state, true);
    } else if (pairFuncs.isFlushOptFunction(FD)) {
      return handleFlush(CE, state, false);
    } else if (pairFuncs.isVfenceFunction(FD)) {
      return handleFence(CE, state, false);
    } else if (pairFuncs.isPfenceFunction(FD)) {
      return handleFence(CE, state, true);
    }

    return false;
  }

  bool handleStmt(const Stmt* S, AbstractState& state) {
    if (const BinaryOperator* BO = dyn_cast<BinaryOperator>(S)) {
      printStmt(S, "bo");
      return handleWrite(BO, state);
    } else if (const CallExpr* CE = dyn_cast<CallExpr>(S)) {
      printStmt(S, "ce");
      return handleCall(CE, state);
    }

    return false;
  }

  void reportBugs(const DataflowResults& allResults, AnalysisManager& mgr,
                  BugReporter& BR) {

    /*
    for (auto& [context, results] : allResults) {
      for (auto& [pl, state] : results) {
        std::set<LatVar> seenVars;
        for (auto& [latVar, latVal] : state) {
          if (seenVars.count(latVar)) {
            continue;
          }

          seenVars.insert(latVar);

          auto PL = pairVars.getPairSet(latVar);
          for (auto PI : PL) {
            LatVar pairLatVar = PI->getPairND(latVar);
            if (!state.count(pairLatVar)) {
              continue;
            }

            seenVars.insert(pairLatVar);
            LatVal pairLatVal = state.find(pairLatVar)->second;

            if (pairLatVal.isWriteFlush() && latVal.isWriteFlush()) {
              llvm::errs() << "bug\n";
            }
          }
        }
      }
    }
    */
  }
};

} // namespace clang::ento::nvm