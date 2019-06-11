#pragma once
#include "Common.h"
#include <cassert>
#include <vector>

namespace clang::ento::nvm {

class DclValue {
protected:
  enum Dcl { WriteDcl, Flush, Pfence, UnseenDcl };
  static const constexpr char* DclStr[] = {"Write", "Flush", "Pfence",
                                           "Unseen"};
  Dcl dcl;

public:
  DclValue(Dcl dcl_) : dcl(dcl_) {}
  DclValue() : dcl(UnseenDcl) {}
};

class SclValue {
protected:
  enum Scl { WriteScl, Vfence, UnseenScl };
  static const constexpr char* SclStr[] = {"Write", "Vfence", "Unseen"};

  Scl scl;

public:
  SclValue(Scl scl_) : scl(scl_) {}
  SclValue() : scl(UnseenScl) {}
};

class PairLattice : public DclValue, public SclValue {
  enum LatticeType { DclType, SclType, BothType, None };

  LatticeType latticeType;

  void meetValue(const PairLattice& val) {
    LatticeType type = val.latticeType;
    if (type != DclType) {
      scl = ((int)scl < (int)val.scl) ? scl : val.scl;
    }
    if (type != SclType) {
      dcl = ((int)dcl < (int)val.dcl) ? dcl : val.dcl;
    }
  }

public:
  PairLattice(LatticeType type)
      : DclValue(UnseenDcl), SclValue(UnseenScl), latticeType(type) {
    assert(type < None);
  }

  PairLattice()
      : DclValue(UnseenDcl), SclValue(UnseenScl), latticeType(None) {}

  PairLattice(const PairLattice& val) { *this = val; }

  LatticeType getLatticeType() const { return latticeType; }

  bool isWriteFlush() const {
    return scl == WriteScl || dcl == WriteDcl || dcl == Flush;
  }

  void dump() const {
    assert(latticeType < None);
    if (latticeType == DclType) {
      llvm::errs() << " dcl:" << DclStr[(int)dcl];
    }
    if (latticeType == SclType) {
      llvm::errs() << " scl:" << SclStr[(int)scl];
    }
  }

  static PairLattice getInit(bool isDcl, bool isScl) {
    assert((isDcl || isScl));
    if (isDcl && isScl) {
      return PairLattice(BothType);
    } else if (isDcl) {
      return PairLattice(DclType);
    } else if (isScl) {
      return PairLattice(SclType);
    } else {
      llvm::report_fatal_error("invalid dscl");
      return PairLattice(BothType);
    }
  }

  static PairLattice getUnseenDcl(const PairLattice& val) {
    PairLattice value(val);
    value.dcl = UnseenDcl;
    return value;
  }

  static PairLattice getWrite(const PairLattice& val) {
    PairLattice value(val);
    LatticeType type = value.latticeType;
    if (type != DclType) {
      value.scl = WriteScl;
    }
    if (type != SclType) {
      value.dcl = WriteDcl;
    }
    return value;
  }

  static PairLattice getFlush(const PairLattice& val) {
    PairLattice value(val);
    value.dcl = Flush;
    return value;
  }

  static PairLattice getPfence(const PairLattice& val) {
    PairLattice value(val);
    value.dcl = Pfence;
    return value;
  }

  static PairLattice getUnseenScl(const PairLattice& val) {
    PairLattice value(val);
    value.scl = UnseenScl;
    return value;
  }

  static PairLattice getVfence(const PairLattice& val) {
    PairLattice value(val);
    value.scl = Vfence;
    return value;
  }

  static PairLattice meet(const PairLattice& value) {
    LatticeType type = value.getLatticeType();
    PairLattice newValue(type);
    newValue.meetValue(value);
    return newValue;
  }

  bool isWrite() const { return dcl == WriteDcl || scl == WriteScl; }

  bool isFlush() const { return dcl == Flush; }

  bool operator<(const PairLattice& X) const {
    return latticeType < X.latticeType && dcl < X.dcl && scl < X.scl;
  }

  bool operator==(const PairLattice& X) const {
    return latticeType == X.latticeType && dcl == X.dcl && scl == X.scl;
  }
};

} // namespace clang::ento::nvm