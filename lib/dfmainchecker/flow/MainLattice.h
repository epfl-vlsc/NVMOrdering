#pragma once
#include "Common.h"
#include "DsclState.h"
#include "main_util/Parser.h"

namespace clang::ento::nvm {

class MainLattice {
  MainFncs mainFncs;
  MainVars mainVars;
  

public:
  using LatVar = const NamedDecl*;
  using LatVal = DsclValue;
  using AbstractState = std::map<LatVar, LatVal>;
  using FunctionResults = std::map<ProgramLocation, AbstractState>;
  using DataflowResults = std::map<PlContext, FunctionResults>;

  void initFunction(const FunctionDecl* FD, AbstractState& state) {
    auto& trackSet = mainVars.getTrackSet(FD);
    for (auto trackVar : trackSet) {
      const NamedDecl* var = trackVar.ND;
      auto lv = DsclValue::getInit(trackVar.isDcl, trackVar.isScl);
      state[var] = lv;
    }
  }

  MainFncs& getMainFncs() { return mainFncs; }

  MainVars& getMainVars() { return mainVars; }

  bool isAnalyze(const FunctionDecl* FD) const {
    return mainFncs.isAnalyze(FD);
  }

  void dump() const {
    mainFncs.dump();
    mainVars.dump();
  }

  bool isIpaCall(const Stmt* S) {
    if (const CallExpr* CE = dyn_cast<CallExpr>(S)) {
      const FunctionDecl* calleeFD = CE->getDirectCallee();
      if (!calleeFD)
        return false;

      return !mainFncs.isSkip(calleeFD);
    }

    return false;
  }

  bool handleWrite(const BinaryOperator* BO, AbstractState& state) {
    Expr* LHS = BO->getLHS();
    if (const MemberExpr* ME = dyn_cast<MemberExpr>(LHS)) {
      const ValueDecl* VD = ME->getMemberDecl();
      if (mainVars.isUsedVar(VD)) {
        state[VD] = DsclValue::getWrite(state[VD]);
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
          if (!mainVars.isUsedVar(VD)) {
            return false;
          }

          if (isPfence) {
            state[VD] = DsclValue::getPfence(state[VD]);
          } else {
            state[VD] = DsclValue::getFlush(state[VD]);
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

    if (mainFncs.isFlushFenceFnc(FD)) {
      return handleFlush(CE, state, true);
    } else if (mainFncs.isFlushOptFnc(FD)) {
      return handleFlush(CE, state, false);
    } else if (mainFncs.isVfenceFnc(FD)) {
      return handleFence(CE, state, false);
    } else if (mainFncs.isPfenceFnc(FD)) {
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
    for (auto& [context, results] : allResults) {
      for (auto& [pl, state] : results) {
        std::set<LatVar> seenVars;
        for (auto& [latVar, latVal] : state) {
          if (seenVars.count(latVar)) {
            continue;
          }

          seenVars.insert(latVar);

          auto PL = mainVars.getPairList(latVar);
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
  }
};

} // namespace clang::ento::nvm