#pragma once
#include "identify/AnnotFunction.h"
#include "identify/NamedFunctions.h"

namespace clang::ento::nvm {

class LogFncs {
  static constexpr const char* ANALYZE = "LogCode";

  AnnotFunction analysisFnc;
  LogFunction logFnc;
  TxBegFunction txBegFnc;
  TxEndFunction txEndFnc;

  bool useTx;
  bool checkTx;

public:
  LogFncs() : analysisFnc(ANALYZE), useTx(false), checkTx(false) {}

  bool isAnalysisFunction(const FunctionDecl* FD) const {
    return analysisFnc.inFunctions(FD);
  }

  bool isLogFunction(const FunctionDecl* FD) const {
    return logFnc.inFunctions(FD);
  }

  bool isTxBegFunction(const FunctionDecl* FD) const {
    return txBegFnc.inFunctions(FD);
  }

  bool isTxEndFunction(const FunctionDecl* FD) const {
    return txEndFnc.inFunctions(FD);
  }

  bool isUsedFnc(const FunctionDecl* FD) const {
    return isLogFunction(FD) || isTxBegFunction(FD) || isTxEndFunction(FD);
  }

  void insertIfKnown(const FunctionDecl* FD) {
    analysisFnc.insertIfKnown(FD);
    logFnc.insertIfKnown(FD);
    txBegFnc.insertIfKnown(FD);
    txEndFnc.insertIfKnown(FD);
  }

  void dump() {
    analysisFnc.dump();
    logFnc.dump();
    txBegFnc.dump();
    txEndFnc.dump();
  }

  void useTxMode() { useTx = !txBegFnc.empty() && !txEndFnc.empty(); }

  bool isTxEnabled() { return useTx; }

  bool checkTxMode() { return checkTx; }

  void setTxMode() { checkTx = true; }
};

} // namespace clang::ento::nvm