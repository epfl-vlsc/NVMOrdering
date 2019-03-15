#pragma once
#include "Common.h"
#include "TypeInfos.h"

namespace clang::ento::nvm
{

struct DclState
{
  private:
    enum Kind
    {
        WD,
        FD,
        PD,
        WC,
        FC,
        PC
    } K;
    DataInfo *DI_;
    DclState(Kind kind, DataInfo *DI)
        : K(kind), DI_(DI) {}

  public:
    bool isWriteData() const { return K == WD; }
    bool isFlushData() const { return K == FD; }
    bool isPFenceData() const { return K == PD; }
    bool isWriteCheck() const { return K == WC; }
    bool isFlushCheck() const { return K == FC; }
    bool isPFenceCheck() const { return K == PC; }

    static DclState getWriteData(DataInfo *DI) { return DclState(WD, DI); }
    static DclState getFlushData(DataInfo *DI) { return DclState(FD, DI); }
    static DclState getPFenceData(DataInfo *DI) { return DclState(PD, DI); }
    static DclState getWriteCheck(DataInfo *DI) { return DclState(WC, DI); }
    static DclState getFlushCheck(DataInfo *DI) { return DclState(FC, DI); }
    static DclState getPFenceCheck(DataInfo *DI) { return DclState(PC, DI); }

    bool isSameCheckName(const StringRef &otherCheckName) const
    {
        return DI_->isSameCheckName(otherCheckName);
    }

    DataInfo *getDataInfo() const
    {
        return DI_;
    }

    bool operator==(const DclState &X) const
    {
        return K == X.K;
    }

    void Profile(llvm::FoldingSetNodeID &ID) const
    {
        ID.AddInteger(K);
        ID.AddPointer(DI_);
    }
};

struct SclState
{
  private:
    enum Kind
    {
        WD,
        VD,
        WC
    } K;
    DataInfo *DI_;

    SclState(Kind kind, DataInfo *DI)
        : K(kind), DI_(DI) {}

  public:
    bool isWriteData() const { return K == WD; }
    bool isVFenceData() const { return K == VD; }
    bool isWriteCheck() const { return K == WC; }

    static SclState getWriteData(DataInfo *DI) { return SclState(WD, DI); }
    static SclState getVFenceData(DataInfo *DI) { return SclState(VD, DI); }
    static SclState getWriteCheck(DataInfo *DI) { return SclState(WC, DI); }

    bool operator==(const SclState &X) const
    {
        return K == X.K;
    }

    bool isSameCheckName(const StringRef &otherCheckName) const
    {
        return DI_->isSameCheckName(otherCheckName);
    }

    DataInfo *getDataInfo() const
    {
        return DI_;
    }

    void Profile(llvm::FoldingSetNodeID &ID) const
    {
        ID.AddInteger(K);
        ID.AddPointer(DI_);
    }
};

} // namespace clang::ento::nvm