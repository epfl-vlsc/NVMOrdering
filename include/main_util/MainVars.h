#pragma once
#include "Common.h"
#include "PairInfo.h"

namespace clang::ento::nvm {

struct TrackVar {
  const NamedDecl* ND;
  bool isDcl;
  bool isScl;

public:
  TrackVar(const NamedDecl* ND_, bool isDcl_, bool isScl_)
      : ND(ND_), isDcl(isDcl_), isScl(isScl_) {}

  bool operator==(const TrackVar& X) const { return ND == X.ND; }

  bool operator<(const TrackVar& X) const { return ND < X.ND; }

  void dump() const {
    llvm::errs() << "track: " << ND->getNameAsString();

    if (isDcl) {
      llvm::errs() << " dcl";
    }
    if (isScl) {
      llvm::errs() << " scl";
    }
  }
};

class MainVars {
  using PairList = std::set<PairInfo*>;
  using PairMap = std::map<const NamedDecl*, PairList>;
  using TrackSet = std::set<TrackVar>;
  using TrackMap = std::map<const FunctionDecl*, TrackSet>;

  PairMap usedVars;
  TrackMap trackMap;

public:
  PairList& getPairList(const NamedDecl* ND) {
    assert(usedVars.count(ND));
    return usedVars[ND];
  }

  TrackSet& getTrackSet(const FunctionDecl* FD) {
    assert(trackMap.count(FD));
    return trackMap[FD];
  }

  bool isUsedVar(const NamedDecl* ND) { return usedVars.count(ND); }

  void addUsedVar(const NamedDecl* ND, PairInfo* PI) {
    if (!usedVars.count(ND)) {
      // not exist
      usedVars[ND];
    }
    usedVars[ND].insert(PI);
  }

  void addTrackVar(const FunctionDecl* FD, TrackVar trackVar) {
    if (!trackMap.count(FD)) {
      trackMap[FD];
    }

    trackMap[FD].insert(trackVar);
  }

  void dump() {
    llvm::errs() << "vars------------------------\n";
    printMapSet(usedVars, dumpPtr<PairInfo>, "var", "\tpair");
    llvm::errs() << "fnc to vars-----------------\n";
    printMapSet(trackMap, dumpObj<TrackVar>, "fnc", "\tvar");
  }
};

} // namespace clang::ento::nvm