#pragma once
#include "Common.h"
#include <cassert>
#include <vector>

namespace clang::ento::nvm {

class DclValue {
protected:
  enum Dcl { Write, Flush, Pfence, Unseen };
  static const constexpr char* DclStr[] = {"Write", "Flush", "Pfence",
                                           "Unseen"};
  Dcl dcl;

public:
  DclValue(Dcl dcl_) : dcl(dcl_) {}
  DclValue() : dcl(Unseen) {}
};

class FlushedLattice : public DclValue {
  void meetValue(const FlushedLattice& val) {
    if (val.dcl < dcl) {
      dcl = val.dcl;
    }
  }

public:
  FlushedLattice() {}

  FlushedLattice(const FlushedLattice& val) { *this = val; }

  void dump() const { llvm::errs() << " dcl:" << DclStr[(int)dcl]; }

  static FlushedLattice getInit() { return FlushedLattice(); }

  static FlushedLattice getWrite(const FlushedLattice& val) {
    FlushedLattice value(val);
    value.dcl = Write;
    return value;
  }

  static FlushedLattice getFlush(const FlushedLattice& val) {
    FlushedLattice value(val);
    value.dcl = Flush;
    return value;
  }

  static FlushedLattice getPfence(const FlushedLattice& val) {
    FlushedLattice value(val);
    value.dcl = Pfence;
    return value;
  }

  static FlushedLattice meet(const FlushedLattice& value) {
    FlushedLattice newValue;
    newValue.meetValue(value);
    return newValue;
  }

  bool isWrite() const { return dcl == Write; }

  bool isFlush() const { return dcl == Flush; }

  bool isWriteFlush() const { return dcl == Write || dcl == Flush; }

  bool isPfence() const { return dcl == Pfence; }

  bool operator<(const FlushedLattice& X) const { return dcl < X.dcl; }

  bool operator==(const FlushedLattice& X) const { return dcl == X.dcl; }
};

} // namespace clang::ento::nvm