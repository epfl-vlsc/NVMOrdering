//===-- OrderingChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "TxMChecker.h"

namespace clang::ento::nvm {

void TxMChecker::checkBeginFunction(CheckerContext& C) const {
  bool isPFnc = nvmTxInfo.isPFunction(C);
  bool isTopFnc = isTopFunction(C);

  // if pmalloc or pfree function, do not analyze
  if (isPFnc && isTopFnc) {
    ExplodedNode* ErrNode = C.generateErrorNode();
    if (!ErrNode)
      return;
  }
}

void TxMChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                                   CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  const MemRegion* Region = Loc.getAsRegion();

  const MemRegion* BR = Region->getBaseRegion();

  bool isNvm = State->get<PMap>(BR);
  if (isNvm) {
    unsigned txCount = State->get<TxCounter>();
    if (txCount == 0) {
      ExplodedNode* ErrNode = C.generateNonFatalErrorNode();
      if (!ErrNode) {
        return;
      }
      BReporter.reportOutTxWriteBug(BR, C, ErrNode, C.getBugReporter());
    }
  }
}

void TxMChecker::checkASTDecl(const FunctionDecl* D,
                                      AnalysisManager& Mgr,
                                      BugReporter& BR) const {
  nvmTxInfo.insertFunction(D);
}

void TxMChecker::checkDeadSymbols(SymbolReaper& SymReaper,
                                          CheckerContext& C) const {
  // llvm::outs() << "checkDeadSymbols\n";
}

ProgramStateRef TxMChecker::checkPointerEscape(
    ProgramStateRef State, const InvalidatedSymbols& Escaped,
    const CallEvent* Call, PointerEscapeKind Kind) const {
  // llvm::outs() << "checkPointerEscape\n";

  return State;
}

void TxMChecker::checkPostCall(const CallEvent& Call,
                                       CheckerContext& C) const {
  const FunctionDecl* FD = getFuncDecl(Call);
  if (nvmTxInfo.isTxBeg(FD)) {
    handleTxBegin(C);
  } else if (nvmTxInfo.isTxEnd(FD)) {
    handleTxEnd(C);
  } else if (nvmTxInfo.isPalloc(FD)) {
    handlePalloc(Call, C);
  } else if (nvmTxInfo.isPfree(FD)) {
    handlePfree(Call, C);
  } else {
    // nothing
  }
}

void TxMChecker::handlePalloc(const CallEvent& Call,
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
    ExplodedNode* ErrNode = C.generateNonFatalErrorNode();
    if (!ErrNode) {
      return;
    }
    BReporter.reportOutTxWriteBug(Region, C, ErrNode, C.getBugReporter());
  }

  State = State->set<PMap>(Region, true);
  C.addTransition(State);
}

void TxMChecker::handlePfree(const CallEvent& Call,
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
      ExplodedNode* ErrNode = C.generateNonFatalErrorNode();
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

void TxMChecker::handleTxBegin(CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  unsigned txCount = State->get<TxCounter>();
  txCount += 1;
  State = State->set<TxCounter>(txCount);
  C.addTransition(State);
}

void TxMChecker::handleTxEnd(CheckerContext& C) const {
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
  registry.addChecker<clang::ento::nvm::TxMChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
