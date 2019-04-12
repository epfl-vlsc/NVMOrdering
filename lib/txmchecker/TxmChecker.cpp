//===-- OrderingChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "TxmChecker.h"

namespace clang::ento::nvm {

void TxmChecker::checkBeginFunction(CheckerContext& C) const {
  bool isPFnc = txDecls.isPFunction(C);
  bool isTopFnc = isTopFunction(C);

  // if pmalloc or pfree function, do not analyze
  if (isPFnc && isTopFnc) {
    ExplodedNode* ErrNode = C.generateErrorNode();
    if (!ErrNode)
      return;
  }
}

void TxmChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                                   CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  const MemRegion* Region = Loc.getAsRegion();

  const MemRegion* BR = Region->getBaseRegion();

  bool isNvm = State->get<PMap>(BR);
  if (isNvm) {
    unsigned txCount = State->get<TxCounter>();
    if (txCount == 0) {
      ExplodedNode* ErrNode = C.generateErrorNode();
      if (!ErrNode) {
        return;
      }
      BReporter.reportOutTxWriteBug(BR, C, ErrNode, C.getBugReporter());
    }
  }
}

void TxmChecker::checkASTDecl(const FunctionDecl* D,
                                      AnalysisManager& Mgr,
                                      BugReporter& BR) const {
  txDecls.insertFunction(D);
}

void TxmChecker::checkDeadSymbols(SymbolReaper& SymReaper,
                                          CheckerContext& C) const {
  // llvm::outs() << "checkDeadSymbols\n";
}

ProgramStateRef TxmChecker::checkPointerEscape(
    ProgramStateRef State, const InvalidatedSymbols& Escaped,
    const CallEvent* Call, PointerEscapeKind Kind) const {
  // llvm::outs() << "checkPointerEscape\n";

  return State;
}

void TxmChecker::checkPostCall(const CallEvent& Call,
                                       CheckerContext& C) const {
  const FunctionDecl* FD = getFuncDecl(Call);
  if (txDecls.isTxBeg(FD)) {
    handleTxBegin(C);
  } else if (txDecls.isTxEnd(FD)) {
    handleTxEnd(C);
  } else if (txDecls.isPalloc(FD)) {
    handlePalloc(Call, C);
  } else if (txDecls.isPfree(FD)) {
    handlePfree(Call, C);
  } else {
    // nothing
  }
}

void TxmChecker::handlePalloc(const CallEvent& Call,
                                      CheckerContext& C) const {
  // taint
  ProgramStateRef State = C.getState();
  SVal RV = Call.getReturnValue();
  const MemRegion* Region = RV.getAsRegion();

  // taint regions
  // todo might lead to bugs due to underlying implementation, this is not how
  // it is used

  unsigned txCount = State->get<TxCounter>();
  if (txCount == 0) {
    ExplodedNode* ErrNode = C.generateErrorNode();
    if (!ErrNode) {
      return;
    }
    BReporter.reportOutTxWriteBug(Region, C, ErrNode, C.getBugReporter());
  }

  State = State->set<PMap>(Region, true);
  C.addTransition(State);
}

void TxmChecker::handlePfree(const CallEvent& Call,
                                     CheckerContext& C) const {
  if (Call.getNumArgs() > 1) {
    llvm::report_fatal_error("check pfree function");
    return;
  }

  ProgramStateRef State = C.getState();
  SVal Loc = Call.getArgSVal(0);
  const MemRegion* Region = Loc.getAsRegion();

  const MemRegion* BR = Region->getBaseRegion();

  bool isNvm = State->get<PMap>(BR);
  if (isNvm) {
    // check transaction
    unsigned txCount = State->get<TxCounter>();
    if (txCount == 0) {
      ExplodedNode* ErrNode = C.generateErrorNode();
      if (!ErrNode) {
        return;
      }
      BReporter.reportOutTxWriteBug(BR, C, ErrNode, C.getBugReporter());
    }

    // clear the region
    State = State->set<PMap>(BR, false);
    C.addTransition(State);
  }
}

void TxmChecker::handleTxBegin(CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  unsigned txCount = State->get<TxCounter>();
  txCount += 1;
  State = State->set<TxCounter>(txCount);
  C.addTransition(State);
}

void TxmChecker::handleTxEnd(CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  unsigned txCount = State->get<TxCounter>();
  if (txCount) {
    // todo report bug that begin does not match end
  }
  txCount -= 1;
  State = State->set<TxCounter>(txCount);
  C.addTransition(State);
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::TxmChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}