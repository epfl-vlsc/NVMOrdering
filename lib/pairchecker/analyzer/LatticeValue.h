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

class LatticeValue : public DclValue, public SclValue {
  enum LatticeType { DclType, SclType, BothType, None };

  LatticeType latticeType;

  void meetValue(const LatticeValue& val) {
    LatticeType type = val.latticeType;
    if (type != DclType) {
      scl = ((int)scl < (int)val.scl) ? scl : val.scl;
    }
    if (type != SclType) {
      dcl = ((int)dcl < (int)val.dcl) ? dcl : val.dcl;
    }
  }

public:
  LatticeValue(LatticeType type)
      : DclValue(UnseenDcl), SclValue(UnseenScl), latticeType(type) {
    assert(type < None);
  }

  LatticeValue() : DclValue(UnseenDcl), SclValue(UnseenScl), latticeType(None) {}

  LatticeValue(const LatticeValue& val) { *this = val; }

  LatticeType getLatticeType() const { return latticeType; }

  bool isWriteFlush() const{
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

  static LatticeValue getInit(bool isDcl, bool isScl) {
    assert((isDcl || isScl));
    if (isDcl && isScl) {
      return LatticeValue(BothType);
    } else if (isDcl) {
      return LatticeValue(DclType);
    } else if (isScl) {
      return LatticeValue(SclType);
    } else {
      llvm::report_fatal_error("invalid dscl");
      return LatticeValue(BothType);
    }
  }

  static LatticeValue getUnseenDcl(const LatticeValue& val) {
    LatticeValue value(val);
    value.dcl = UnseenDcl;
    return value;
  }

  static LatticeValue getWrite(const LatticeValue& val) {
    LatticeValue value(val);
    LatticeType type = value.latticeType;
    if (type != DclType) {
      value.scl = WriteScl;
    }
    if (type != SclType) {
      value.dcl = WriteDcl;
    }
    return value;
  }

  static LatticeValue getFlush(const LatticeValue& val) {
    LatticeValue value(val);
    value.dcl = Flush;
    return value;
  }

  static LatticeValue getPfence(const LatticeValue& val) {
    LatticeValue value(val);
    value.dcl = Pfence;
    return value;
  }

  static LatticeValue getUnseenScl(const LatticeValue& val) {
    LatticeValue value(val);
    value.scl = UnseenScl;
    return value;
  }

  static LatticeValue getVfence(const LatticeValue& val) {
    LatticeValue value(val);
    value.scl = Vfence;
    return value;
  }

  static LatticeValue meet(const LatticeValue& value) {
    LatticeType type = value.getLatticeType();
    LatticeValue newValue(type);
    newValue.meetValue(value);
    return newValue;
  }

  bool operator<(const LatticeValue& X) const {
    return latticeType < X.latticeType && dcl < X.dcl && scl < X.scl;
  }

  bool operator==(const LatticeValue& X) const {
    return latticeType == X.latticeType && dcl == X.dcl && scl == X.scl;
  }
};

} // namespace clang::ento::nvm