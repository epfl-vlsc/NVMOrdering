#pragma once
#include "Common.h"

namespace clang::ento::nvm {

class PairInfo;

struct WriteState {
private:
  enum Kind { WD, FD, PD, WC, FC, PC } K;
  static constexpr const char* Str[] = {"WriteData", "FlushData", "PfenceData",
                                        "WriteCheck", "FlushCheck", "PfenceCheck"};
  static constexpr const char* Exp[] = {"data is written", "data is flushed",
                                        "data is pfenced", "check is written",
                                        "check is flushed", "check if pfenced"};

  WriteState(Kind K_) : K(K_) {}

public:
  bool isWriteData() const { return K == WD; }
  bool isFlushData() const { return K == FD; }
  bool isPfenceData() const { return K == PD; }
  bool isWriteCheck() const { return K == WC; }
  bool isFlushCheck() const { return K == FC; }
  bool isPfenceCheck() const { return K == PC; }

  bool isFinal() const { return K == PC; }
  static WriteState getInit() { return WriteState(WD); }

  static WriteState getWriteData() { return WriteState(WD); }
  static WriteState getFlushData() { return WriteState(FD); }
  static WriteState getPfenceData() { return WriteState(PD); }
  static WriteState getWriteCheck() { return WriteState(WC); }
  static WriteState getFlushCheck() { return WriteState(FC); }
  static WriteState getPfenceCheck() { return WriteState(PC); }

  int getState() const { return static_cast<int>(K); }

  bool operator==(const WriteState& X) const { return K == X.K; }

  const char* getStateName() const { return Str[K]; }
  const char* getExplanation() const { return Exp[K]; }

  void Profile(llvm::FoldingSetNodeID& ID) const {
    ID.AddInteger(K);
    ID.AddPointer(Str);
    ID.AddPointer(Exp);
  }
};

} // namespace clang::ento::nvm

REGISTER_MAP_WITH_PROGRAMSTATE(WriteMap, const clang::ento::nvm::PairInfo*,
                               clang::ento::nvm::WriteState)
