#pragma once
#include "Common.h"

namespace clang::ento::nvm {
/*
struct DclState {
private:
  enum Kind { WD, FD, PD, WC } K;
  static constexpr const char* Str[] = {"WriteData", "FlushData", "PfenceData",
                                        "WriteCheck"};

  DclState(Kind K_) : K(K_) {}

public:
  bool isWriteData() const { return K == WD; }
  bool isFlushData() const { return K == FD; }
  bool isPFenceData() const { return K == PD; }
  bool isWriteCheck() const { return K == WC; }

  static DclState getWriteData() { return DclState(WD); }
  static DclState getFlushData() { return DclState(FD); }
  static DclState getPFenceData() { return DclState(PD); }
  static DclState getWriteCheck() { return DclState(WC); }

  int getState() const { return static_cast<int>(K); }

  bool operator==(const DclState& X) const { return K == X.K; }

  const char* getStateName() const { return Str[K]; }

  void Profile(llvm::FoldingSetNodeID& ID) const {
    ID.AddInteger(K);
    ID.AddPointer(Str);
  }
};

struct SclState {
private:
  enum Kind { WD, VD, WC } K;
  static constexpr const char* Str[] = {"WriteData", "VfenceData",
                                        "WriteCheck"};

  SclState(Kind kind) : K(kind) {}

public:
  bool isWriteData() const { return K == WD; }
  bool isVFenceData() const { return K == VD; }
  bool isWriteCheck() const { return K == WC; }

  static SclState getWriteData() { return SclState(WD); }
  static SclState getVFenceData() { return SclState(VD); }
  static SclState getWriteCheck() { return SclState(WC); }

  int getStateKind() const { return static_cast<int>(K); }

  bool operator==(const SclState& X) const { return K == X.K; }

  const char* getStateName() const { return Str[K]; }

  void Profile(llvm::FoldingSetNodeID& ID) const {
    ID.AddInteger(K);
    ID.AddPointer(Str);
  }
};

struct CheckState {
private:
  enum Kind { WD, FD, PD } K;
  static constexpr const char* Str[] = {"WriteData", "FlushData", "PfenceData"};

  CheckState(Kind kind) : K(kind) {}

public:
  bool isWriteData() const { return K == WD; }
  bool isFlushData() const { return K == FD; }
  bool isPfenceData() const { return K == PD; }

  static CheckState getWriteData() { return CheckState(WD); }
  static CheckState getVFenceData() { return CheckState(FD); }
  static CheckState getWriteCheck() { return CheckState(PD); }

  int getStateKind() const { return static_cast<int>(K); }

  bool operator==(const CheckState& X) const { return K == X.K; }

  const char* getStateName() const { return Str[K]; }

  void Profile(llvm::FoldingSetNodeID& ID) const {
    ID.AddInteger(K);
    ID.AddPointer(Str);
  }
};

struct CheckState {
private:
  enum Kind { WD, FD, PD } K;
  static constexpr const char* Str[] = {"WriteData", "FlushData", "PfenceData"};

  CheckState(Kind kind) : K(kind) {}

public:
  bool isWriteData() const { return K == WD; }
  bool isFlushData() const { return K == FD; }
  bool isPfenceData() const { return K == PD; }

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
*/
} // namespace clang::ento::nvm
/*
REGISTER_MAP_WITH_PROGRAMSTATE(SclMap, const clang::DeclaratorDecl*,
                               clang::ento::nvm::SclState)
REGISTER_MAP_WITH_PROGRAMSTATE(DclMap, const clang::DeclaratorDecl*,
                               clang::ento::nvm::DclState)
                               */