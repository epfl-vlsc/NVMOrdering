#pragma once
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

class DsclValue : public DclValue, public SclValue {
  using DsclVector = std::vector<DsclValue>;
  enum DsclType { DclType, SclType, BothType, None };

  DsclType dsclType;

  void meetValue(const DsclValue& val) {
    DsclType type = val.dsclType;
    if (type != DclType) {
      scl = ((int)scl < (int)val.scl) ? scl : val.scl;
    }
    if (type != SclType) {
      dcl = ((int)dcl < (int)val.dcl) ? dcl : val.dcl;
    }
  }

public:
  DsclValue(DsclType type)
      : DclValue(UnseenDcl), SclValue(UnseenScl), dsclType(type) {
    assert(type < None);
  }

  DsclValue() : DclValue(UnseenDcl), SclValue(UnseenScl), dsclType(None) {}

  DsclValue(const DsclValue& val) { *this = val; }

  DsclType getDsclType() const { return dsclType; }

  void dump() const {
    assert(dsclType < None);
    if (dsclType == DclType) {
      llvm::errs() << " dcl:" << DclStr[(int)dcl];
    }
    if (dsclType == SclType) {
      llvm::errs() << " scl:" << SclStr[(int)scl];
    }
  }

  static DsclValue getInit(bool isDcl, bool isScl) {
    assert((isDcl || isScl));
    if (isDcl && isScl) {
      return DsclValue(BothType);
    } else if (isDcl) {
      return DsclValue(DclType);
    } else if (isScl) {
      return DsclValue(SclType);
    } else {
      llvm::report_fatal_error("invalid dscl");
      return DsclValue(BothType);
    }
  }

  static DsclValue getUnseenDcl(const DsclValue& val) {
    DsclValue value(val);
    value.dcl = UnseenDcl;
    return value;
  }

  static DsclValue getWrite(const DsclValue& val) {
    DsclValue value(val);
    DsclType type = value.dsclType;
    if (type != DclType) {
      value.scl = WriteScl;
    }
    if (type != SclType) {
      value.dcl = WriteDcl;
    }
    return value;
  }

  static DsclValue getFlush(const DsclValue& val) {
    DsclValue value(val);
    value.dcl = Flush;
    return value;
  }

  static DsclValue getPfence(const DsclValue& val) {
    DsclValue value(val);
    value.dcl = Pfence;
    return value;
  }

  static DsclValue getUnseenScl(const DsclValue& val) {
    DsclValue value(val);
    value.scl = UnseenScl;
    return value;
  }

  static DsclValue getVfence(const DsclValue& val) {
    DsclValue value(val);
    value.scl = Vfence;
    return value;
  }

  static DsclValue meet(const DsclVector& values) {
    DsclType type = values[0].getDsclType();
    DsclValue newValue(type);
    for (auto value : values) {
      newValue.meetValue(value);
    }

    return newValue;
  }

  static DsclValue meet(const DsclValue& value) {
    DsclType type = value.getDsclType();
    DsclValue newValue(type);
    newValue.meetValue(value);
    return newValue;
  }

  bool operator<(const DsclValue& X) const {
    return dsclType < X.dsclType && dcl < X.dcl && scl < X.scl;
  }

  bool operator==(const DsclValue& X) const {
    return dsclType == X.dsclType && dcl == X.dcl && scl == X.scl;
  }
};

} // namespace clang::ento::nvm