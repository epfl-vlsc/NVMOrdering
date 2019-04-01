//===-- LogChecker.cpp -----------------------------------------*

#include "LogChecker.h"
#include "Transitions.h"

namespace clang::ento::nvm {

void LogChecker::checkASTDecl(const FunctionDecl* FD, AnalysisManager& Mgr,
                              BugReporter& BR) const {
  logInfos.insertIfKnown(FD);
}

void LogChecker::checkASTDecl(const ValueDecl* VD, AnalysisManager& Mgr,
                              BugReporter& BR) const {
  logInfos.insertIfKnown(VD);
}

void LogChecker::checkPostCall(const CallEvent& Call, CheckerContext& C) const {
  if (logInfos.isCodeFunction(Call)) {
    // handle code
    handleCodeFnc(Call, C);
  } else if (logInfos.isPtrFunction(Call)) {
    // handle ptr
    handlePtrFnc(Call, C);
  }
}

void LogChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                           CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  if (const ValueDecl* VD = getValueDecl(Loc); VD) {
    if (logInfos.isSpecialValue(VD)) {
      writeTransition(VD, State, C, BReporter);
    }
  }
}

void LogChecker::handlePtrFnc(const CallEvent& Call, CheckerContext& C) const {
  if (Call.getNumArgs() > 2) {
    llvm::report_fatal_error("check ptr function");
    return;
  }

  ProgramStateRef State = C.getState();

  SVal Loc = Call.getArgSVal(0);
  if (const ValueDecl* VD = getValueDecl(Loc); VD) {
    if (logInfos.isSpecialValue(VD)) {
      logTransition(VD, State, C, BReporter);
    }
  }
}

void LogChecker::handleCodeFnc(const CallEvent& Call, CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  codeTransition(State, C);
}

/*
void TxPChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                           CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  const MemRegion* Region = Loc.getAsRegion();

  const MemRegion* BR = Region->getBaseRegion();

  const WriteState* WS = State->get<PMap>(BR);
  if (WS) {
    // is a palloced region

    // check tx region
    unsigned txCount = State->get<TxCounter>();
    if (txCount == 0) {
      ExplodedNode* ErrNode = C.generateNonFatalErrorNode();
      if (!ErrNode) {
        return;
      }
      BReporter.reportTxWriteBug(BR, C, ErrNode, C.getBugReporter());
    }

    // check mem usage
    if (WS->isLogged()) {
      State = State->set<PMap>(BR, WriteState::getWritten());
      C.addTransition(State);
    } else {
      // not logged
      ExplodedNode* ErrNode = C.generateNonFatalErrorNode();
      if (!ErrNode) {
        return;
      }
      BReporter.reportTxLogBug(BR, C, ErrNode, C.getBugReporter());
    }
  }
}

void TxPChecker::checkASTDecl(const FunctionDecl* D, AnalysisManager& Mgr,
                              BugReporter& BR) const {
  nvmTxInfo.insertFunction(D);
}

void TxPChecker::checkDeadSymbols(SymbolReaper& SymReaper,
                                  CheckerContext& C) const {
  // llvm::outs() << "checkDeadSymbols\n";
}

ProgramStateRef TxPChecker::checkPointerEscape(
    ProgramStateRef State, const InvalidatedSymbols& Escaped,
    const CallEvent* Call, PointerEscapeKind Kind) const {
  // llvm::outs() << "checkPointerEscape\n";

  return State;
}



void TxPChecker::handleTxAdd(const CallEvent& Call, CheckerContext& C) const {
  if (Call.getNumArgs() > 1) {
    llvm::report_fatal_error("check tx_add function");
    return;
  }

  ProgramStateRef State = C.getState();
  SVal Loc = Call.getArgSVal(0);
  const MemRegion* Region = Loc.getAsRegion();

  const MemRegion* BR = Region->getBaseRegion();

  const WriteState* WS = State->get<PMap>(BR);
  if (WS) {
    // check transaction
    unsigned txCount = State->get<TxCounter>();
    if (txCount == 0) {
      ExplodedNode* ErrNode = C.generateNonFatalErrorNode();
      if (!ErrNode) {
        return;
      }
      BReporter.reportTxWriteBug(BR, C, ErrNode, C.getBugReporter());
    }

    // clear the region
    State = State->set<PMap>(BR, WriteState::getLogged());
    C.addTransition(State);
  }
}

void TxPChecker::handlePalloc(const CallEvent& Call, CheckerContext& C) const {
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
    BReporter.reportTxWriteBug(Region, C, ErrNode, C.getBugReporter());
  }

  State = State->set<PMap>(Region, WriteState::getInit());
  C.addTransition(State);
}

void TxPChecker::handlePfree(const CallEvent& Call, CheckerContext& C) const {
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
      BReporter.reportTxWriteBug(BR, C, ErrNode, C.getBugReporter());
    }

    // clear the region
    State = State->remove<PMap>(BR);
    C.addTransition(State);
  }
}

void TxPChecker::handleTxBegin(CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  unsigned txCount = State->get<TxCounter>();
  txCount += 1;
  State = State->set<TxCounter>(txCount);
  C.addTransition(State);
}

void TxPChecker::handleTxEnd(CheckerContext& C) const {
  ProgramStateRef State = C.getState();
  unsigned txCount = State->get<TxCounter>();
  if (txCount) {
    // todo report bug that begin does not match end
  }
  txCount -= 1;
  State = State->set<TxCounter>(txCount);
  C.addTransition(State);
}
*/

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::LogChecker>(CHECKER_PLUGIN_NAME,
                                                    "Checks logging use");
}
