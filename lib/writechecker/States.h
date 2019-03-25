#pragma once
#include "Common.h"
#include "TypeInfos.h"

namespace clang::ento::nvm {

struct DclState {
private:
  enum Kind { WD, FD, PD, WC, FC, PC } K;
  DataInfo* DI_;
  static constexpr const char* Str[] = {"WriteData",  "FlushData",
                                        "PfenceData", "WriteCheck",
                                        "FlushCheck", "PfenceCheck"};

  DclState(Kind kind, DataInfo* DI) : K(kind), DI_(DI) {}

public:
  bool isWriteData() const { return K == WD; }
  bool isFlushData() const { return K == FD; }
  bool isPFenceData() const { return K == PD; }
  bool isWriteCheck() const { return K == WC; }
  bool isFlushCheck() const { return K == FC; }
  bool isPFenceCheck() const { return K == PC; }

  static DclState getWriteData(DataInfo* DI) { return DclState(WD, DI); }
  static DclState getFlushData(DataInfo* DI) { return DclState(FD, DI); }
  static DclState getPFenceData(DataInfo* DI) { return DclState(PD, DI); }
  static DclState getWriteCheck(DataInfo* DI) { return DclState(WC, DI); }
  static DclState getFlushCheck(DataInfo* DI) { return DclState(FC, DI); }
  static DclState getPFenceCheck(DataInfo* DI) { return DclState(PC, DI); }

  int getStateKind() const { return static_cast<int>(K); }

  bool isSameCheckName(const StringRef& otherCheckName) const {
    return DI_->isSameCheckName(otherCheckName);
  }

  DataInfo* getDataInfo() const { return DI_; }

  bool operator==(const DclState& X) const { return K == X.K; }

  const char* getStateName() const { return Str[K]; }

  void Profile(llvm::FoldingSetNodeID& ID) const {
    ID.AddInteger(K);
    ID.AddPointer(DI_);
    ID.AddPointer(Str);
  }
};

struct SclState {
private:
  enum Kind { WD, VD, WC } K;
  DataInfo* DI_;
  static constexpr const char* Str[] = {"WriteData", "VfenceData",
                                        "WriteCheck"};

  SclState(Kind kind, DataInfo* DI) : K(kind), DI_(DI) {}

public:
  bool isWriteData() const { return K == WD; }
  bool isVFenceData() const { return K == VD; }
  bool isWriteCheck() const { return K == WC; }

  static SclState getWriteData(DataInfo* DI) { return SclState(WD, DI); }
  static SclState getVFenceData(DataInfo* DI) { return SclState(VD, DI); }
  static SclState getWriteCheck(DataInfo* DI) { return SclState(WC, DI); }

  int getStateKind() const { return static_cast<int>(K); }

  bool operator==(const SclState& X) const { return K == X.K; }

  bool isSameCheckName(const StringRef& otherCheckName) const {
    return DI_->isSameCheckName(otherCheckName);
  }

  DataInfo* getDataInfo() const { return DI_; }

  const char* getStateName() const { return Str[K]; }

  void Profile(llvm::FoldingSetNodeID& ID) const {
    ID.AddInteger(K);
    ID.AddPointer(DI_);
    ID.AddPointer(Str);
  }
};

} // namespace clang::ento::nvm

REGISTER_MAP_WITH_PROGRAMSTATE(SclMap, const clang::DeclaratorDecl*,
                               clang::ento::nvm::SclState)
REGISTER_MAP_WITH_PROGRAMSTATE(DclMap, const clang::DeclaratorDecl*,
                               clang::ento::nvm::DclState)