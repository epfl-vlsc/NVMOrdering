#pragma once
#include "Common.h"

namespace clang::ento::nvm {

struct DclState {
private:
  enum Kind { WD, FD, PD, WC } K;
  static constexpr const char* Str[] = {"WriteData", "FlushData", "PfenceData",
                                        "WriteCheck"};
  static constexpr const char* Exp[] = {"data is written", "data is flushed",
                                        "data is pfenced", "check is written"};

  DclState(Kind K_) : K(K_) {}

public:
  bool isWriteData() const { return K == WD; }
  bool isFlushData() const { return K == FD; }
  bool isPfenceData() const { return K == PD; }
  bool isWriteCheck() const { return K == WC; }

  bool isFinal() const { return K == WC; }
  static DclState getInit() { return DclState(WD); }

  static DclState getWriteData() { return DclState(WD); }
  static DclState getFlushData() { return DclState(FD); }
  static DclState getPfenceData() { return DclState(PD); }
  static DclState getWriteCheck() { return DclState(WC); }

  int getState() const { return static_cast<int>(K); }

  bool operator==(const DclState& X) const { return K == X.K; }

  const char* getStateName() const { return Str[K]; }
  const char* getExplanation() const { return Exp[K]; }

  void Profile(llvm::FoldingSetNodeID& ID) const {
    ID.AddInteger(K);
    ID.AddPointer(Str);
  }
};

} // namespace clang::ento::nvm

REGISTER_MAP_WITH_PROGRAMSTATE(DclMap, const char*, clang::ento::nvm::DclState)
