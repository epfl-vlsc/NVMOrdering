#pragma once
#include "Common.h"

namespace clang::ento::nvm {

struct CheckState {
private:
  enum Kind { WD, FD, PD } K;
  static constexpr const char* Str[] = {"WriteData", "FlushData", "PfenceData"};

  CheckState(Kind kind) : K(kind) {}

public:
  bool isWriteData() const { return K == WD; }
  bool isFlushData() const { return K == FD; }
  bool isPfenceData() const { return K == PD; }

  bool isFinal() const { return K == PD; }
  static CheckState getInit() { return CheckState(WD); }

  static CheckState getWriteData() { return CheckState(WD); }
  static CheckState getFlushData() { return CheckState(FD); }
  static CheckState getPfenceData() { return CheckState(PD); }

  int getStateKind() const { return static_cast<int>(K); }

  bool operator==(const CheckState& X) const { return K == X.K; }

  const char* getStateName() const { return Str[K]; }

  void Profile(llvm::FoldingSetNodeID& ID) const {
    ID.AddInteger(K);
    ID.AddPointer(Str);
  }
};

} // namespace clang::ento::nvm

REGISTER_MAP_WITH_PROGRAMSTATE(CheckMap, const char*, clang::ento::nvm::CheckState)
