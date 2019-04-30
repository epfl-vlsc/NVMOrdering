#pragma once
#include "Common.h"

namespace clang::ento::nvm {

struct WriteState {
private:
  enum Kind { WD, FD, PD, WC } K;
  static constexpr const char* Str[] = {"WriteData", "FlushData", "PfenceData",
                                        "WriteCheck"};
  static constexpr const char* Exp[] = {"data is written", "data is flushed",
                                        "data is pfenced", "check is written"};

  WriteState(Kind K_) : K(K_) {}

public:
  bool isWriteData() const { return K == WD; }
  bool isFlushData() const { return K == FD; }
  bool isPfenceData() const { return K == PD; }
  bool isWriteCheck() const { return K == WC; }

  bool isFinal() const { return K == WC; }
  static WriteState getInit() { return WriteState(WD); }

  static WriteState getWriteData() { return WriteState(WD); }
  static WriteState getFlushData() { return WriteState(FD); }
  static WriteState getPfenceData() { return WriteState(PD); }
  static WriteState getWriteCheck() { return WriteState(WC); }

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

REGISTER_MAP_WITH_PROGRAMSTATE(WriteMap, const clang::NamedDecl*,
                               clang::ento::nvm::WriteState)
