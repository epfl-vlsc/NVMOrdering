#pragma once
#include "Common.h"
#include "dataflow_util/AbstractProgram.h"
#include "dataflow_util/TransitionChange.h"
#include "parser_util/ParseUtils.h"

namespace clang::ento::nvm {

template <typename Variables, typename Functions> class PairTransitions {
  using FunctionInfo = typename Functions::FunctionInfo;
  using LatVar = const NamedDecl*;
  using LatVal = PairLattice;
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
  BugReporter* BR;
  const CheckerBase* CB;

  // for bug finding
  using PairSet = std::set<const NamedDecl*>;
  using VarToPair = std::map<const NamedDecl*, PairSet>;
  using LastLocationMap = std::map<const NamedDecl*, const AbstractLocation*>;
  VarToPair* varToPair;
  LastLocationMap* lastLocationMap;
  std::unique_ptr<BugType> CommitBug;

  // for creating abstract graph
  bool useGlobal;

  void updateLastLocation(const NamedDecl* ND,
                          const AbstractLocation* absLocation) {
    (*lastLocationMap)[ND] = absLocation;
  }

  PairSet& getPairSet(const NamedDecl* ND) {
    auto& pairSet = (*varToPair)[ND];

    auto& PISet = vars->getPairInfoSet(ND);
    for (auto& PI : PISet) {
      const NamedDecl* pairND = PI->getPairND(ND);
      if (activeUnitInfo->isUsedVar(pairND)) {
        pairSet.insert(pairND);
      }
    }
    return pairSet;
  }

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
  TransitionChange handleWrite(const AbstractStmt* absStmt,
                               const PairTransitionInfo& PTI,
                               AbstractState& state) {
    const NamedDecl* ND = PTI.getND();
    state[ND] = LatVal::getWrite(state[ND]);

    auto& pairSet = getPairSet(ND);
    for (auto& pairND : pairSet) {
      auto& pairLattice = state[pairND];
      if (pairLattice.isWriteFlush()) {
        printMsg("bug here");
        assert(lastLocationMap);
        const AbstractLocation* absLocation = (*lastLocationMap)[pairND];
        assert(absLocation);
        absLocation->fullDump(Mgr);
        printMsg("");
        absStmt->fullDump(Mgr);
        printMsg("");

        SmallString<100> buf;
        llvm::raw_svector_ostream os(buf);
        os << "Error writing " << ND->getQualifiedNameAsString() << " commit "
           << pairND->getQualifiedNameAsString() << "\n";

        auto* Ctx = Mgr->getAnalysisDeclContext(ND);
        PathDiagnosticLocation L = PathDiagnosticLocation::createBegin(
            absStmt->getStmt(), Mgr->getSourceManager(), Ctx);
        auto R = llvm::make_unique<BugReport>(*CommitBug, os.str(), L);
        R->addRange(absLocation->getSourceRange());
        R->addRange(absStmt->getSourceRange());
        BR->emitReport(std::move(R));

        return TransitionChange::BugChange;
      }
    }

    updateLastLocation(ND, absStmt);
    return TransitionChange::StateChange;
  }

  TransitionChange handleFence(const AbstractStmt* absStmt,
                               const PairTransitionInfo& PTI,
                               AbstractState& state) {
    llvm::report_fatal_error("not implemented");
    return TransitionChange::NoChange;
  }

  TransitionChange handleFlush(const AbstractStmt* absStmt,
                               const PairTransitionInfo& PTI,
                               AbstractState& state) {
    const NamedDecl* ND = PTI.getND();
    bool isPfence = PTI.isPfence();

    auto& LV = state[ND];

    if (!LV.isWrite()) {
      return TransitionChange::NoChange;
    }

    if (isPfence) {
      state[ND] = LatVal::getPfence(LV);
    } else {
      state[ND] = LatVal::getFlush(LV);
    }

    updateLastLocation(ND, absStmt);
    return TransitionChange::StateChange;
  }

  TransitionChange handleStmt(const AbstractStmt* absStmt,
                              const PairTransitionInfo& PTI,
                              AbstractState& state) {
    switch (PTI.getTransferFunction()) {
    case PairTransitionInfo::WriteFunc:
      return handleWrite(absStmt, PTI, state);
    case PairTransitionInfo::FlushFunc:
      return handleFlush(absStmt, PTI, state);
    case PairTransitionInfo::FlushFenceFunc:
      return handleFlush(absStmt, PTI, state);
    case PairTransitionInfo::VfenceFunc:
      return handleFence(absStmt, PTI, state);
    case PairTransitionInfo::PfenceFunc:
      return handleFence(absStmt, PTI, state);
    default:
      return TransitionChange::NoChange;
    }
  }

public:
  void initAll(Variables& vars_, Functions& funcs_, AnalysisManager* Mgr_,
               BugReporter* BR_, const CheckerBase* CB_) {
    vars = &vars_;
    funcs = &funcs_;
    Mgr = Mgr_;
    BR = BR_;
    useGlobal = false;
    varToPair = nullptr;
    lastLocationMap = nullptr;
    CB = CB_;
    CommitBug.reset(new BugType(CB, "Not committed", ""));
  }

  void useGlobalTransitions() { useGlobal = true; }

  void initUnit(FunctionInfo* activeUnitInfo_) {
    activeUnitInfo = activeUnitInfo_;
    useGlobal = false;

    // to find pairs quickly
    if (varToPair) {
      delete varToPair;
    }
    varToPair = new VarToPair;
    if (lastLocationMap) {
      delete lastLocationMap;
    }
    lastLocationMap = new LastLocationMap();
  }

  void initLatticeValues(AbstractState& state) {
    for (auto& usedVar : activeUnitInfo->getUsedVars()) {
      auto& [isDcl, isScl] = vars->getDsclInfo(usedVar);
      auto lv = LatVal::getInit(isDcl, isScl);
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

  TransitionChange handleStmt(const AbstractStmt* absStmt,
                              AbstractState& state) {
    // parse stmt to usable structure
    const Stmt* S = absStmt->getStmt();

    auto PTI = parseStmt(S);

    // handle stmt
    return handleStmt(absStmt, PTI, state);
  }

  // access structures
  auto getAnalysisFunctions() { return funcs->getAnalysisFunctions(); }

  AnalysisManager* getMgr() { return Mgr; }
};

} // namespace clang::ento::nvm
