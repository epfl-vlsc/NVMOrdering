#pragma once
#include "Common.h"
#include "DsclState.h"
#include "DsclTransfer.h"
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

  void handleStmt(const Stmt* S, AbstractState& state, FunctionResults& results) {
    if (const BinaryOperator* BO = dyn_cast<BinaryOperator>(S)) {
      printStmt(S, "bo");
      if (BO->isAssignmentOp()) {
        Expr* LHS = BO->getLHS();
        if (const MemberExpr* ME = dyn_cast<MemberExpr>(LHS)) {
          const ValueDecl* VD = ME->getMemberDecl();

          state[VD] = DsclValue::getWrite(state[VD]);
          ProgramLocation plStmt(BO);
          results[plStmt] = state;
        }
      }
    } else if (const CallExpr* CE = dyn_cast<CallExpr>(S)) {
      printStmt(S, "ce");
      const FunctionDecl* calleeFD = CE->getDirectCallee();
      if (!calleeFD)
        return;

      IdentifierInfo* FnInfo = calleeFD->getIdentifier();

      if (FnInfo->isStr("clflush")) {
        printMsg("clflush");
        const Expr* arg0 = CE->getArg(0);

        if (const ImplicitCastExpr* ICE = dyn_cast<ImplicitCastExpr>(arg0)) {
          const Stmt* Child1 = getNthChild(ICE, 0);
          if (const UnaryOperator* UO = dyn_cast<UnaryOperator>(Child1)) {
            const Stmt* expr = UO->getSubExpr();
            if (const MemberExpr* ME = dyn_cast<MemberExpr>(expr)) {
              const ValueDecl* VD = ME->getMemberDecl();

              state[VD] = DsclValue::getPfence(state[VD]);
              ProgramLocation plStmt(CE);
              results[plStmt] = state;
            }
          }
        }

        printStmt(arg0, "arg");
      }
    }
  }
};

} // namespace clang::ento::nvm