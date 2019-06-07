#pragma once
#include "Common.h"
#include "PairInfo.h"

namespace clang::ento::nvm {

class PairVariables {
  using PairSet = std::set<PairInfo*>;
  using PairMap = std::map<const NamedDecl*, PairSet>;

  PairMap usedVars;

public:
  PairSet& getPairSet(const NamedDecl* ND) {
    assert(usedVars.count(ND));
    return usedVars[ND];
  }

  bool isUsedVar(const NamedDecl* ND) { return usedVars.count(ND); }

  void addUsedVar(const NamedDecl* ND, PairInfo* PI) {
    if (!usedVars.count(ND)) {
      // not exist
      usedVars[ND];
    }
    usedVars[ND].insert(PI);
  }

  void dump() const {
    llvm::errs() << "vars------------------------\n";
    printMapSet(usedVars, dumpPtr<PairInfo>, "var", "\tpair");
  }
};
} // namespace clang::ento::nvm