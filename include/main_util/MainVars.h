#pragma once
#include "Common.h"
#include "PairInfo.h"

namespace clang::ento::nvm {

class MainVars {
  using PairList = std::set<PairInfo*>;
  std::map<const NamedDecl*, PairList> usedVars;

public:
  PairList& getPairList(const NamedDecl* ND) {
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

  void dump() {
    llvm::errs() << "vars------------------------\n";
    printMapSet(usedVars, "nd", "\tpair");
  }
};

} // namespace clang::ento::nvm