#pragma once
#include "Common.h"
#include <cassert>
#include <vector>

namespace clang::ento::nvm {

class TxValue {
protected:
  using Tx = int;
  Tx tx;

public:
  TxValue(int tx_) : tx(tx_) {}
  TxValue() : tx(0) {}
};

class LogValue {
protected:
  enum Log { Logged, Unseen };
  static const constexpr char* LogStr[] = {"Logged", "Unseen"};

  Log log;

public:
  LogValue(Log log_) : log(log_) {}
  LogValue() : log(Unseen) {}
};

class LogLattice : public TxValue, public LogValue {
  enum LatticeType { TxValueType, LogValueType, None };

  LatticeType latticeType;

  void meetValue(const LogLattice& val) {
    LatticeType type = val.latticeType;
    if (type == TxValueType && val.tx < tx) {
      tx = val.tx;
    }
    if (type == LogValueType && (int)val.log < (int)log) {
      log = val.log;
    }
  }

public:
  LogLattice(LatticeType type) : latticeType(type) { assert(type < None); }

  LogLattice() : latticeType(None) {}

  LogLattice(const LogLattice& val) { *this = val; }

  LatticeType getLatticeType() const { return latticeType; }

  void dump() const {
    assert(latticeType < None);
    if (latticeType == TxValueType) {
      llvm::errs() << " tx:" << tx;
    }
    if (latticeType == LogValueType) {
      llvm::errs() << " log:" << LogStr[(int)log];
    }
  }

  static LogLattice getInitTx() {
    return LogLattice(TxValueType);
  }

  static LogLattice getInitVar() {
    return LogLattice(LogValueType);
  }

  static LogLattice getBeginTx(const LogLattice& val) {
    LogLattice value(val);
    value.tx++;
    return value;
  }

  static LogLattice getEndTx(const LogLattice& val) {
    LogLattice value(val);
    value.tx--;
    return value;
  }

  static LogLattice getLogged(const LogLattice& val) {
    LogLattice value(val);
    value.log = Logged;
    return value;
  }

  static LogLattice meet(const LogLattice& value) {
    LatticeType type = value.getLatticeType();
    LogLattice newValue(type);
    newValue.meetValue(value);
    return newValue;
  }

  bool isLogged() const { return log == Logged; }

  bool isInTx() const { return tx > 0; }

  bool operator<(const LogLattice& X) const {
    switch (latticeType) {
    case TxValueType:
      return tx < X.tx;
    case LogValueType:
      return log < X.log;
    default: {
      llvm::report_fatal_error("lattice value does not have a type");
      return false;
    }
    }
  }

  bool operator==(const LogLattice& X) const {
    switch (latticeType) {
    case TxValueType:
      return tx == X.tx;
    case LogValueType:
      return log == X.log;
    default: {
      llvm::report_fatal_error("lattice value does not have a type");
      return false;
    }
    }
  }
};

} // namespace clang::ento::nvm