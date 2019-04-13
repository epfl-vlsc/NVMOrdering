#pragma once
#include "Common.h"

namespace clang::ento::nvm {

struct SclState {
private:
  enum Kind { WD, VD, WC } K;
  static constexpr const char* Str[] = {"WriteData", "VfenceData",
                                        "WriteCheck"};
  static constexpr const char* Exp[] = {"data is written", "data is vfenced",
                                        "check is written"};

  SclState(Kind kind) : K(kind) {}

public:
  bool isWriteData() const { return K == WD; }
  bool isVfenceData() const { return K == VD; }
  bool isWriteCheck() const { return K == WC; }

  bool isFinal() const { return K == WC || K == VD; }
  static SclState getInit() { return SclState(WD); }

  static SclState getWriteData() { return SclState(WD); }
  static SclState getVfenceData() { return SclState(VD); }
  static SclState getWriteCheck() { return SclState(WC); }

  int getStateKind() const { return static_cast<int>(K); }

  bool operator==(const SclState& X) const { return K == X.K; }

  const char* getStateName() const { return Str[K]; }
  const char* getExplanation() const { return Exp[K]; }

  void Profile(llvm::FoldingSetNodeID& ID) const {
    ID.AddInteger(K);
    ID.AddPointer(Str);
    ID.AddPointer(Exp);
  }
};

} // namespace clang::ento::nvm

REGISTER_MAP_WITH_PROGRAMSTATE(SclMap, const char*, clang::ento::nvm::SclState)
