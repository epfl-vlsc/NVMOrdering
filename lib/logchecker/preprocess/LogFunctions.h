#pragma once
#include "Common.h"
#include "global_util/AnnotFunction.h"
#include "global_util/NamedFunctions.h"

namespace clang::ento::nvm {

class LogFunctions {
public:
  using FunctionSet = std::set<const FunctionDecl*>;

protected:
  static constexpr const char* LOG_FNC = "LogCode";
  static constexpr const char* SKIP_FNC = "SkipCode";

  AnnotFunction analyzedFnc;
  AnnotFunction skipFnc;
  LogFunction logFnc;
  TxBegFunction txBeginFnc;
  TxEndFunction txEndFnc;

  bool useTx;

public:
  LogFunctions() : analyzedFnc(LOG_FNC), skipFnc(SKIP_FNC), useTx(false) {}

  auto& getAnalyzedFunctions() { return analyzedFnc; }

  bool isSkipFunction(const FunctionDecl* FD) const {
    return isLogFunction(FD) || isTxBeginFunction(FD) || isTxEndFunction(FD) ||
           skipFnc.inFunctions(FD);
  }

  bool isAnalyzedFunction(const FunctionDecl* FD) const {
    return analyzedFnc.inFunctions(FD);
  }

  bool isLogFunction(const FunctionDecl* FD) const {
    return logFnc.inFunctions(FD);
  }

  bool isTxBeginFunction(const FunctionDecl* FD) const {
    return txBeginFnc.inFunctions(FD);
  }

  bool isTxEndFunction(const FunctionDecl* FD) const {
    return txEndFnc.inFunctions(FD);
  }

  void insertFunction(const FunctionDecl* FD) {
    analyzedFnc.insertFunction(FD);
    skipFnc.insertFunction(FD);
    logFnc.insertFunction(FD);
    txBeginFnc.insertFunction(FD);
    txEndFnc.insertFunction(FD);
  }

  void dumpFunctions() const {
    analyzedFnc.dump();
    skipFnc.dump();
    logFnc.dump();
    txBeginFnc.dump();
    txEndFnc.dump();
  }

  void setTxMode() {
    useTx = !txBeginFnc.empty() && !txEndFnc.empty();
    if (useTx) {
      assert(txBeginFnc.size() == 1 && txEndFnc.size() == 1 &&
             "must have one type of tx");
    }
  }

  bool isUseTx() const { return useTx; }

  const NamedDecl* getTxFunction() const {
    auto first = txBeginFnc.begin();
    return *first;
  }
};

} // namespace clang::ento::nvm