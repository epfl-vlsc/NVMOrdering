#pragma once
#include "Common.h"
#include "PairInfo.h"

namespace clang::ento::nvm {

class PairVariables {
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

  using PairInfoSet = std::set<PairInfo*>;
  using PairInfoMap = std::map<const NamedDecl*, PairInfoSet>;
  using VarInfo = std::map<const NamedDecl*, DsclInfo>;

  VarInfo usedVars;
  PairInfoMap pairInfoMap;

public:
  PairInfoSet& getPairInfoSet(const NamedDecl* ND) {
    assert(pairInfoMap.count(ND));
    return pairInfoMap[ND];
  }

  DsclInfo& getDsclInfo(const NamedDecl* ND) {
    assert(usedVars.count(ND));
    return usedVars[ND];
  }

  bool isUsedVar(const NamedDecl* ND) { return usedVars.count(ND); }

  void addUsedVar(const NamedDecl* ND, PairInfo* PI) {
    if (!usedVars.count(ND)) {
      // not exist
      usedVars[ND];
      pairInfoMap[ND];
    }

    //insert var information
    auto& dsclInfo = usedVars[ND];
    dsclInfo.isDcl |= !PI->isSameCl();
    dsclInfo.isScl |= PI->isSameCl();

    //insert pair information
    pairInfoMap[ND].insert(PI);
  }

  void dump() const {
    llvm::errs() << "vars------------------------\n";
    for (auto& [ND, dsclInfo] : usedVars) {
      auto& pairInfoSet = pairInfoMap.find(ND)->second;
      printND(ND, "var", true, false);
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