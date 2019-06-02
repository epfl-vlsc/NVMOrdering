#pragma once
#include <cassert>
#include <vector>

namespace clang::ento::nvm {

class DsclValue {
  using DsclVector = std::vector<DsclValue>;
  enum DsclType { DclType, SclType, BothType };
  enum Dcl { UnseenDcl, WriteDcl, Flush, Pfence };
  enum Scl { UnseenScl, WriteScl, Vfence };

  DsclType dsclType;
  Dcl dcl;
  Scl scl;

  DsclValue(DsclType type) : dcl(UnseenDcl), scl(UnseenScl), dsclType(type) {
    assert(type < 3);
  }

  DsclValue(const DsclValue& val) { *this = val; }

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
  static DsclValue getInitial(DsclType type) { return DsclValue(type); }

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
    DsclType type = values[0].dsclType;
    DsclValue newValue(type);
    for (auto value : values) {
      newValue.meetValue(value);
    }

    return newValue;
  }
};

} // namespace clang::ento::nvm