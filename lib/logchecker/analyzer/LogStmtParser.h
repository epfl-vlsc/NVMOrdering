#pragma once
#include "Common.h"
#include "parser_util/ParseUtils.h"

namespace clang::ento::nvm {

template <typename Globals> class LogStmtParser {
  using LatVar = typename Globals::LatVar;

  class LogTransitionInfo {
  public:
    enum TransferFunction {
      WriteFunc,
      LogFunc,
      TxBeginFunc,
      TxEndFunc,
      CalleeFunc,
      NoFunc
    };

    static const constexpr char* tfStr[6] = {"Write", "Log",    "TxBegin",
                                             "TxEnd", "Callee", "No"};

  private:
    TransferFunction transferFunc;
    LatVar var;

  public:
    LogTransitionInfo() : transferFunc(NoFunc), var(nullptr) {}
    LogTransitionInfo(TransferFunction transferFunc_, LatVar var_)
        : transferFunc(transferFunc_), var(var_) {
      assert(var);
    }
    LogTransitionInfo(TransferFunction transferFunc_)
        : transferFunc(transferFunc_), var(nullptr) {}

    void dump() const {
      if (transferFunc == NoFunc) {
        llvm::errs() << "Skip stmt\n";
      } else {
        llvm::errs() << tfStr[(int)transferFunc] << " function";
        printND(var, "");
      }
    }

    static LogTransitionInfo getCallee() {
      return LogTransitionInfo(CalleeFunc);
    }

    bool isStmtUsed() const { return transferFunc != NoFunc; }

    LatVar getVar() const { return var; }

    TransferFunction getTransferFunction() const { return transferFunc; }

    static TransferFunction getWriteFunction() { return WriteFunc; }

    static TransferFunction getLogFunction() { return LogFunc; }

    static TransferFunction getTxBeginFunction() { return TxBeginFunc; }

    static TransferFunction getTxEndFunction() { return TxEndFunc; }
  };

  // parse-------------------------------------------------------
  auto parseWrite(const BinaryOperator* BO) {
    const ValueDecl* VD = ParseUtils::getFieldDeclFromWrite(BO);
    if (!globals.isUsedVar(VD)) {
      return LogTransitionInfo();
    }

    auto transferFunction = LogTransitionInfo::getWriteFunction();
    return LogTransitionInfo(transferFunction, VD);
  }

  auto parseLog(const CallExpr* CE) {
    const ValueDecl* VD = ParseUtils::getFieldDeclFromCall(CE);
    if (!globals.isUsedVar(VD)) {
      return LogTransitionInfo();
    }

    auto transferFunction = LogTransitionInfo::getLogFunction();
    return LogTransitionInfo(transferFunction, VD);
  }

  auto parseTxBegin(const CallExpr* CE) {
    auto transferFunction = LogTransitionInfo::getTxBeginFunction();
    return LogTransitionInfo(transferFunction);
  }

  auto parseTxEnd(const CallExpr* CE) {
    auto transferFunction = LogTransitionInfo::getTxEndFunction();
    return LogTransitionInfo(transferFunction);
  }

  auto parseCall(const CallExpr* CE) {
    const FunctionDecl* FD = CE->getDirectCallee();
    if (!FD)
      return LogTransitionInfo();

    if (globals.isLogFunction(FD)) {
      return parseLog(CE);
    } else if (globals.isTxBeginFunction(FD)) {
      return parseTxBegin(CE);
    } else if (globals.isTxEndFunction(FD)) {
      return parseTxEnd(CE);
    }

    return LogTransitionInfo();
  }

  Globals& globals;

public:
  using TransitionInfo = LogTransitionInfo;

  LogStmtParser(Globals& globals_) : globals(globals_) {}

  auto parseStmt(const Stmt* S) {
    if (const BinaryOperator* BO = dyn_cast<BinaryOperator>(S)) {
      return parseWrite(BO);
    } else if (const CallExpr* CE = dyn_cast<CallExpr>(S)) {
      return parseCall(CE);
    }

    return LogTransitionInfo();
  }
};

} // namespace clang::ento::nvm
