#pragma once
#include "Common.h"


namespace clang::ento::nvm{



struct DCLState{
private:
    enum Kind{WD, FD, PD, WC, FC, PC} K;
    DCLState(Kind kind):K(kind){}

public:
    bool isWriteData() const {return K == WD;}
    bool isFlushData() const {return K == FD;}
    bool isPFenceData() const {return K == PD;}
    bool isWriteCheck() const {return K == WC;}
    bool isFlushCheck() const {return K == FC;}
    bool isPFenceCheck() const {return K == PC;}

    static DCLState getWriteData() { return DCLState(WD); }
    static DCLState getFlushData() { return DCLState(FD); }
    static DCLState getPFenceData() { return DCLState(PD); }
    static DCLState getWriteCheck() { return DCLState(WC); }
    static DCLState getFlushCheck() { return DCLState(FC); }
    static DCLState getPFenceCheck() { return DCLState(PC); }

    bool operator==(const DCLState &X) const {
        return K == X.K;
    }

    void Profile(llvm::FoldingSetNodeID &ID) const {
        ID.AddInteger(K);
    }
};

struct SCLState{
private:
    enum Kind{WD, VD, WC} K;
    SCLState(Kind kind):K(kind){}

public:
    bool isWriteData() const {return K == WD;}
    bool isVFenceData() const {return K == VD;}
    bool isWriteCheck() const {return K == WC;}
    
    static SCLState getWriteData() { return SCLState(WD); }
    static SCLState getVFenceData() { return SCLState(VD); }
    static SCLState getWriteCheck() { return SCLState(WC); }
    
    bool operator==(const SCLState &X) const {
        return K == X.K;
    }

    void Profile(llvm::FoldingSetNodeID &ID) const {
        ID.AddInteger(K);
    }
};





} //namespace nvm