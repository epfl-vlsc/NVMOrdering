#pragma once
#include "Common.h"

namespace clang::ento::nvm::SlSpace {

struct SlRange {
private:
  SourceRange SR;

  SlRange(SourceRange SR_) : SR(SR_) {}

public:
  static SlRange getSlRange(SourceRange SR_) {
    return SlRange(SR_);
  }

  SourceRange getSR() { return SR; }

  bool operator==(const SlRange& X) const { return SR == (X.SR); }

  void Profile(llvm::FoldingSetNodeID& ID) const {  }
};

} // namespace clang::ento::nvm::SlSpace

REGISTER_LIST_WITH_PROGRAMSTATE(TransitionStore,
                                clang::ento::nvm::SlSpace::SlRange)

namespace clang::ento::nvm::SlSpace {

void saveSR(ProgramStateRef& State, const SourceRange& SR) {
  State = State->add<TransitionStore>(SlRange::getSlRange(SR));
}

TransitionStoreTy getSRStore(const ProgramStateRef& State) {
  TransitionStoreTy LineStore = State->get<TransitionStore>();
  return LineStore;
}

} // namespace clang::ento::nvm::SlSpace
