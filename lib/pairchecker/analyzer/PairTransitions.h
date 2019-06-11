#pragma once
#include "Common.h"
#include "PairTransfer.h"
#include "parser_util/ParseUtils.h"

namespace clang::ento::nvm {

template <typename Variables, typename Functions> class PairTransitions {
  using FunctionInfo = typename Functions::FunctionInfo;
  using LatVar = const NamedDecl*;
  using LatVal = LatticeValue;
  using AbstractState = std::map<LatVar, LatVal>;

  class PairTransitionInfo {
  public:
    enum TransferFunction {
      WriteFunc,
      FlushFunc,
      FlushFenceFunc,
      VfenceFunc,
      PfenceFunc,
      IpaFunc,
      NoFunc
    };

    static const constexpr char* tfStr[7] = {
        "Write", "Flush", "FlushFence", "Vfence", "Pfence", "Ipa", "No"};

  private:
    TransferFunction transferFunc;
    const NamedDecl* ND;

    PairTransitionInfo(TransferFunction transferFunc_)
        : transferFunc(transferFunc_), ND(nullptr) {}

  public:
    PairTransitionInfo() : transferFunc(NoFunc), ND(nullptr) {}
    PairTransitionInfo(TransferFunction transferFunc_, const NamedDecl* ND_)
        : transferFunc(transferFunc_), ND(ND_) {
      assert(ND);
    }

    void dump() const {
      if (transferFunc == NoFunc) {
        llvm::errs() << "Skip stmt\n";
      } else {
        llvm::errs() << tfStr[(int)transferFunc] << " function";
        printND(ND, "");
      }
    }

    static PairTransitionInfo getIpa() { return PairTransitionInfo(IpaFunc); }

    bool isStmtUsed() const { return transferFunc != NoFunc; }

    const NamedDecl* getND() const { return ND; }

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

  // useful structures
  Variables* vars;
  Functions* funcs;
  FunctionInfo* activeUnitInfo;
  AnalysisManager* Mgr;

  // for creating abstract graph
  bool useGlobal;

  // parse-------------------------------------------------------
  bool isUsedVar(const NamedDecl* ND) {
    if (useGlobal) {
      return vars->isUsedVar(ND);
    } else {
      assert(activeUnitInfo && "active unit must be set");
      return activeUnitInfo->isUsedVar(ND);
    }
  }

  PairTransitionInfo parseWrite(const BinaryOperator* BO) {
    const MemberExpr* ME = ParseUtils::getME(BO);
    if (!ME) {
      return PairTransitionInfo();
    }

    const ValueDecl* VD = ME->getMemberDecl();
    if (!isUsedVar(VD)) {
      return PairTransitionInfo();
    }

    auto transferFunction = PairTransitionInfo::getWriteFunction();
    return PairTransitionInfo(transferFunction, VD);
  }

  PairTransitionInfo parseFlush(const CallExpr* CE, bool isPfence) {
    const MemberExpr* ME = ParseUtils::getME(CE);
    if (!ME) {
      return PairTransitionInfo();
    }

    const ValueDecl* VD = ME->getMemberDecl();
    if (!isUsedVar(VD)) {
      return PairTransitionInfo();
    }

    auto transferFunction = PairTransitionInfo::getFlushFunction(isPfence);
    return PairTransitionInfo(transferFunction, VD);
  }

  PairTransitionInfo parseFence(const CallExpr* CE, bool isPfence) {
    llvm::report_fatal_error("not implemented");
    return PairTransitionInfo();
  }

  PairTransitionInfo parseCall(const CallExpr* CE) {
    const FunctionDecl* FD = CE->getDirectCallee();
    if (!FD)
      return PairTransitionInfo();

    if (funcs->isFlushFenceFunction(FD)) {
      return parseFlush(CE, true);
    } else if (funcs->isFlushOptFunction(FD)) {
      return parseFlush(CE, false);
    } else if (funcs->isPfenceFunction(FD)) {
      return parseFence(CE, true);
    } else if (funcs->isVfenceFunction(FD)) {
      return parseFence(CE, false);
    } else if (useGlobal && !funcs->isSkipFunction(FD)) {
      return PairTransitionInfo::getIpa();
    }

    return PairTransitionInfo();
  }

  // handle----------------------------------------------------
  bool handleWrite(const PairTransitionInfo& PTI, AbstractState& state) {
    PTI.dump();
    return transferWrite(PTI.getND(), state);
  }

  bool handleFence(const PairTransitionInfo& PTI, AbstractState& state) {
    llvm::report_fatal_error("not implemented");
    return false;
  }

  bool handleFlush(const PairTransitionInfo& PTI, AbstractState& state) {
    return transferFlush(PTI.getND(), state, PTI.isPfence());
  }

  bool handleStmt(const PairTransitionInfo& PTI, AbstractState& state) {
    switch (PTI.getTransferFunction()) {
    case PairTransitionInfo::WriteFunc:
      return handleWrite(PTI, state);
      break;
    case PairTransitionInfo::FlushFunc:
      return handleFlush(PTI, state);
    case PairTransitionInfo::FlushFenceFunc:
      return handleFlush(PTI, state);
    case PairTransitionInfo::VfenceFunc:
      return handleFence(PTI, state);
    case PairTransitionInfo::PfenceFunc:
      return handleFence(PTI, state);
    default:
      return false;
    }
  }

public:
  void initAll(Variables& vars_, Functions& funcs_, AnalysisManager* Mgr_) {
    vars = &vars_;
    funcs = &funcs_;
    Mgr = Mgr_;
    useGlobal = false;
  }

  void useGlobalTransitions() { useGlobal = true; }

  void initUnit(FunctionInfo* activeUnitInfo_) {
    activeUnitInfo = activeUnitInfo_;
    useGlobal = false;
  }

  void initLatticeValues(AbstractState& state) {
    for (auto& usedVar : activeUnitInfo->getUsedVars()) {
      auto& [isDcl, isScl] = vars->getDsclInfo(usedVar);
      auto lv = LatticeValue::getInit(isDcl, isScl);
      state[usedVar] = lv;
    }
  }

  PairTransitionInfo parseStmt(const Stmt* S) {
    if (const BinaryOperator* BO = dyn_cast<BinaryOperator>(S)) {
      return parseWrite(BO);
    } else if (const CallExpr* CE = dyn_cast<CallExpr>(S)) {
      return parseCall(CE);
    }

    return PairTransitionInfo();
  }

  bool handleStmt(const Stmt* S, AbstractState& state) {
    // parse stmt to usable structure
    auto PTI = parseStmt(S);

    // handle stmt 
    return handleStmt(PTI, state);
  }

  // access structures
  auto getAnalysisFunctions() { return funcs->getAnalysisFunctions(); }

  AnalysisManager* getMgr() { return Mgr; }
};

} // namespace clang::ento::nvm
