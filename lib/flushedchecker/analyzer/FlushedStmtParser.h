#pragma once
#include "Common.h"
#include "parser_util/ParseUtils.h"

namespace clang::ento::nvm {

template <typename Globals> class FlushedStmtParser {
  using LatVar = typename Globals::LatVar;

  class FlushedTransitionInfo {
  public:
    enum TransferFunction {
      WriteFunc,
      FlushFunc,
      FlushFenceFunc,
      PfenceFunc,
      CalleeFunc,
      NoFunc
    };

    enum AccessType { FieldAccess, TrackedVariableAccess, NoAccess };

    static const constexpr char* tfStr[6] = {"Write",  "Flush", "FlushFence",
                                             "Pfence", "Ipa",   "No"};

    static const constexpr char* accStr[3] = {"Field", "TrackedVariable", "No"};

  private:
    TransferFunction transferFunc;
    AccessType accessType;
    LatVar fieldVariable;
    LatVar trackedVariable;

  public:
    FlushedTransitionInfo(TransferFunction transferFunc_,
                          AccessType accessType_, LatVar fieldVariable_,
                          LatVar trackedVariable_)
        : transferFunc(transferFunc_), accessType(accessType_),
          fieldVariable(fieldVariable_), trackedVariable(trackedVariable_) {}

    FlushedTransitionInfo()
        : transferFunc(NoFunc), accessType(NoAccess), fieldVariable(nullptr),
          trackedVariable(nullptr) {}

    void dump() const {
      if (transferFunc == NoFunc) {
        llvm::errs() << "Skip stmt\n";
      } else {
        llvm::errs() << tfStr[(int)transferFunc] << " function";
      }
    }

    static FlushedTransitionInfo getCallee() {
      return FlushedTransitionInfo(CalleeFunc);
    }

    bool isStmtUsed() const { return transferFunc != NoFunc; }

    LatVar getFieldVariable() const { return fieldVariable; }

    LatVar getTrackedVariable() const { return trackedVariable; }

    AccessType getAccessType() const { return accessType; }

    TransferFunction getTransferFunction() const { return transferFunc; }

    bool isPfence() const {
      return transferFunc == FlushFenceFunc || transferFunc == PfenceFunc;
    }

    static TransferFunction getWriteFunction() { return WriteFunc; }

    static TransferFunction getFlushFunction(bool isPfence) {
      return (isPfence) ? FlushFenceFunc : FlushFunc;
    }

    static TransferFunction getFenceFunction() { return PfenceFunc; }

    static FlushedTransitionInfo
    getTrackedVariableFTI(TransferFunction transferFunc_,
                          LatVar trackedVariable_) {
      return FlushedTransitionInfo(transferFunc_, TrackedVariableAccess,
                                   nullptr, trackedVariable_);
    }

    static FlushedTransitionInfo getFieldFTI(TransferFunction transferFunc_,
                                             LatVar fieldVariable,
                                             LatVar trackedVariable_) {
      return FlushedTransitionInfo(transferFunc_, FieldAccess, trackedVariable_,
                                   trackedVariable_);
    }

    static FlushedTransitionInfo getFTI(TransferFunction transferFunc_) {
      return FlushedTransitionInfo(transferFunc_, NoAccess, nullptr, nullptr);
    }

    static FlushedTransitionInfo getFTI() { return FlushedTransitionInfo(); }
  };

  // parse-------------------------------------------------------
  auto parseWrite(const BinaryOperator* BO) {
    auto field = ParseUtils::getFieldDeclFromWrite(BO);
    auto rhs = ParseUtils::getNDOfRHS(BO);
    if (field && rhs && globals.isFieldVariable(field) &&
        globals.isTrackedVariable(rhs)) {
      auto transferFunction = FlushedTransitionInfo::getWriteFunction();
      return FlushedTransitionInfo::getFieldFTI(transferFunction, field, rhs);
    }

    auto trackedVarLHS = ParseUtils::getNDOfAssigned(BO);
    if (trackedVarLHS && globals.isTrackedVariable(trackedVarLHS)) {
      auto transferFunction = FlushedTransitionInfo::getWriteFunction();
      return FlushedTransitionInfo::getTrackedVariableFTI(transferFunction,
                                                          trackedVarLHS);
    }

    return FlushedTransitionInfo::getFTI();
  }

  auto parseFlush(const CallExpr* CE, bool isPfence) {
    /*
    auto arg0 = CE->getArg(0);
    printStmt(arg0, "arg0");
    */

    auto trackedVar = ParseUtils::getPtrFromFlush(CE);
    /*
    if(trackedVar)
    trackedVar->dump();
    */

    if (trackedVar && globals.isTrackedVariable(trackedVar)) {
      auto transferFunction = FlushedTransitionInfo::getFlushFunction(isPfence);
      return FlushedTransitionInfo::getTrackedVariableFTI(transferFunction,
                                                          trackedVar);
    }

    return FlushedTransitionInfo::getFTI();
  }

  auto parseFence(const CallExpr* CE) {
    auto transferFunction = FlushedTransitionInfo::getFenceFunction();
    return FlushedTransitionInfo::getFTI(transferFunction);
  }

  auto parseCall(const CallExpr* CE) {
    const FunctionDecl* FD = CE->getDirectCallee();
    if (!FD)
      return FlushedTransitionInfo();

    if (globals.isFlushFenceFunction(FD)) {
      return parseFlush(CE, true);
    } else if (globals.isFlushOptFunction(FD)) {
      return parseFlush(CE, false);
    } else if (globals.isPfenceFunction(FD)) {
      return parseFence(CE);
    }

    return FlushedTransitionInfo::getFTI();
  }

  Globals& globals;

public:
  using TransitionInfo = FlushedTransitionInfo;

  FlushedStmtParser(Globals& globals_) : globals(globals_) {}

  auto parseStmt(const Stmt* S) {
    if (const BinaryOperator* BO = dyn_cast<BinaryOperator>(S)) {
      return parseWrite(BO);
    } else if (const CallExpr* CE = dyn_cast<CallExpr>(S)) {
      return parseCall(CE);
    }

    return FlushedTransitionInfo();
  }
};

} // namespace clang::ento::nvm
