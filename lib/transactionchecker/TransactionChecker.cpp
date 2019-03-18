//===-- OrderingChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "TransactionChecker.h"

namespace clang::ento::nvm {

void TransactionChecker::checkBeginFunction(CheckerContext& C) const {
  bool isPFnc = nvmTxInfo.isPFunction(C);
  bool isTopFnc = isTopFunction(C);

  // if top special function, do not analyze
  if (isPFnc && isTopFnc) {
    llvm::outs() << "skip analysis\n";
    ExplodedNode* ErrNode = C.generateErrorNode();
    if (!ErrNode)
      return;
  }
}

void TransactionChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                                   CheckerContext&) const {
  /*
  llvm::outs() << "checkBind\n";
  llvm::outs() << "Loc\n";
  Loc.dump();
  llvm::outs() << "\nVal\n";
  Val.dump();
  llvm::outs() << "\n";
  S->dump();
  llvm::outs() << "\n";
  */
}

void TransactionChecker::checkASTDecl(const FunctionDecl* D,
                                      AnalysisManager& Mgr,
                                      BugReporter& BR) const {
  nvmTxInfo.insertFunction(D);
}

void TransactionChecker::checkDeadSymbols(SymbolReaper& SymReaper,
                                          CheckerContext& C) const {
  // llvm::outs() << "checkDeadSymbols\n";
}

ProgramStateRef TransactionChecker::checkPointerEscape(
    ProgramStateRef State, const InvalidatedSymbols& Escaped,
    const CallEvent* Call, PointerEscapeKind Kind) const {
  // llvm::outs() << "checkPointerEscape\n";

  return State;
}

void TransactionChecker::checkPostCall(const CallEvent &Call, CheckerContext &C) const{
  const FunctionDecl* FD = getFuncDecl(Call);
  if(nvmTxInfo.isTxBeg(FD)){
    handleTxBegin(C);
  }else if(nvmTxInfo.isTxEnd(FD)){
    handleTxEnd(C);
  }else{
    //nothing
  }
}

void TransactionChecker::handleTxBegin(CheckerContext& C) const{
  ProgramStateRef State = C.getState();
  unsigned txCount = State->get<TxCounter>();
  txCount += 1;
  State = State->set<TxCounter>(txCount);
  C.addTransition(State);
}

void TransactionChecker::handleTxEnd(CheckerContext& C) const{
  ProgramStateRef State = C.getState();
  unsigned txCount = State->get<TxCounter>();
  if(txCount){
    //todo report bug that begin does not match end
  }
  txCount -= 1;
  State = State->set<TxCounter>(txCount);
  C.addTransition(State);
}


} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::TransactionChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
