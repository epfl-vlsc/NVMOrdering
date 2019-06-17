#pragma once
#include "Common.h"
#include "PairInfo.h"

namespace clang::ento::nvm {

class PairVariables {
protected:
  struct DsclInfo {
    bool isDcl;
    bool isScl;

    DsclInfo() : isDcl(false), isScl(false) {}

    void dump() const {
      if (isDcl) {
        llvm::errs() << " dcl";
      }
      if (isScl) {
        llvm::errs() << " scl";
      }
    }
  };

  using LatVar = const NamedDecl*;
  using VariableSet = std::set<LatVar>;
  using PairInfoSet = std::set<PairInfo*>;
  using PairInfoMap = std::map<LatVar, PairInfoSet>;
  using VarInfo = std::map<LatVar, DsclInfo>;

  VarInfo usedVars;
  PairInfoMap pairInfoMap;

public:
  PairInfoSet& getPairInfoSet(LatVar var) {
    assert(pairInfoMap.count(var));
    return pairInfoMap[var];
  }

  DsclInfo& getDsclInfo(LatVar var) {
    assert(usedVars.count(var));
    return usedVars[var];
  }

  bool isUsedVar(LatVar var) { return usedVars.count(var); }

  void insertVariable(LatVar var, PairInfo* PI) {
    if (!usedVars.count(var)) {
      // not exist
      usedVars[var];
      pairInfoMap[var];
    }

    // insert var information
    auto& dsclInfo = usedVars[var];
    dsclInfo.isDcl |= !PI->isSameCl();
    dsclInfo.isScl |= PI->isSameCl();

    // insert pair information
    pairInfoMap[var].insert(PI);
  }

  void dumpVariables() const {
    llvm::errs() << "vars------------------------\n";
    for (auto& [var, dsclInfo] : usedVars) {
      auto& pairInfoSet = pairInfoMap.find(var)->second;
      printND(var, "var", true, false);
      dsclInfo.dump();
      printMsg("");
      for (auto& PI : pairInfoSet) {
        printMsg("\t", false);
        PI->dump();
        printMsg(" - ", false);
      }
      printMsg("");
    }
  }
};
} // namespace clang::ento::nvm