#pragma once
#include "Common.h"
#include "parser_util/ParseUtils.h"

namespace clang::ento::nvm {

template <typename Globals> class PairStmtParser {
  using LatVar = typename Globals::LatVar;

  class PairTransitionInfo {
  public:
    enum TransferFunction {
      WriteFunc,
      FlushFunc,
      FlushFenceFunc,
      VfenceFunc,
      PfenceFunc,
      CalleeFunc,
      NoFunc
    };

    static const constexpr char* tfStr[7] = {
        "Write", "Flush", "FlushFence", "Vfence", "Pfence", "Ipa", "No"};

  private:
    TransferFunction transferFunc;
    LatVar var;

  public:
    PairTransitionInfo() : transferFunc(NoFunc), var(nullptr) {}
    PairTransitionInfo(TransferFunction transferFunc_, LatVar var_)
        : transferFunc(transferFunc_), var(var_) {
      assert(var);
    }
    PairTransitionInfo(TransferFunction transferFunc_)
        : transferFunc(transferFunc_), var(nullptr) {}

    void dump() const {
      if (transferFunc == NoFunc) {
        llvm::errs() << "Skip stmt\n";
      } else {
        llvm::errs() << tfStr[(int)transferFunc] << " function";
        printND(var, "");
      }
    }

    static PairTransitionInfo getCallee() {
      return PairTransitionInfo(CalleeFunc);
    }

    bool isStmtUsed() const { return transferFunc != NoFunc; }

    const NamedDecl* getVar() const { return var; }

    TransferFunction getTransferFunction() const { return transferFunc; }

    bool isPfence() const {
      return transferFunc == FlushFenceFunc || transferFunc == PfenceFunc;
    }

    static TransferFunction getWriteFunction() { return WriteFunc; }

    static TransferFunction getFlushFunction(bool isPfence) {
      return (isPfence) ? FlushFenceFunc : FlushFunc;
    }

    static TransferFunction getFenceFunction(bool isPfence) {
      return (isPfence) ? PfenceFunc : VfenceFunc;
    }
  };

  // parse-------------------------------------------------------
  auto parseWrite(const BinaryOperator* BO) {
    const MemberExpr* ME = ParseUtils::getME(BO);
    if (!ME) {
      return PairTransitionInfo();
    }

    const ValueDecl* VD = ME->getMemberDecl();
    if (!globals.isUsedVar(VD)) {
      return PairTransitionInfo();
    }

    auto transferFunction = PairTransitionInfo::getWriteFunction();
    return PairTransitionInfo(transferFunction, VD);
  }

  auto parseFlush(const CallExpr* CE, bool isPfence) {
    const MemberExpr* ME = ParseUtils::getME(CE);
    if (!ME) {
      return PairTransitionInfo();
    }

    const ValueDecl* VD = ME->getMemberDecl();
    if (!globals.isUsedVar(VD)) {
      return PairTransitionInfo();
    }

    auto transferFunction = PairTransitionInfo::getFlushFunction(isPfence);
    return PairTransitionInfo(transferFunction, VD);
  }

  auto parseFence(const CallExpr* CE, bool isPfence) {
    auto transferFunction = PairTransitionInfo::getFenceFunction(isPfence);
    return PairTransitionInfo(transferFunction);
  }

  auto parseCall(const CallExpr* CE) {
    const FunctionDecl* FD = CE->getDirectCallee();
    if (!FD)
      return PairTransitionInfo();

    if (globals.isFlushFenceFunction(FD)) {
      return parseFlush(CE, true);
    } else if (globals.isFlushOptFunction(FD)) {
      return parseFlush(CE, false);
    } else if (globals.isPfenceFunction(FD)) {
      return parseFence(CE, true);
    } else if (globals.isVfenceFunction(FD)) {
      return parseFence(CE, false);
    }

    return PairTransitionInfo();
  }

  Globals& globals;

public:
  using TransitionInfo = PairTransitionInfo;

  PairStmtParser(Globals& globals_) : globals(globals_) {}

  auto parseStmt(const Stmt* S) {
    if (const BinaryOperator* BO = dyn_cast<BinaryOperator>(S)) {
      return parseWrite(BO);
    } else if (const CallExpr* CE = dyn_cast<CallExpr>(S)) {
      return parseCall(CE);
    }

    return PairTransitionInfo();
  }
};

} // namespace clang::ento::nvm
