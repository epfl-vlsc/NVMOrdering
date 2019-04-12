#pragma once

#include "Common.h"
#include "StateInfo.h"
#include "Transitions.h"
#include "walkers/Walkers.h"

namespace clang::ento::nvm {

class BaseInfo {
protected:
  const ValueDecl* data;
  BaseInfo(const ValueDecl* data_) : data(data_) {}

public:
  virtual void dump() const { llvm::outs() << data->getNameAsString(); }

  virtual bool useData(StateInfo& SI) const = 0;
  virtual void write(StateInfo& SI) const = 0;
  virtual void flush(StateInfo& SI) const = 0;
  virtual void pfence(StateInfo& SI) const = 0;
  virtual void vfence(StateInfo& SI) const = 0;
};

class CheckInfo : public BaseInfo {

public:
  CheckInfo(const ValueDecl* data_) : BaseInfo(data_) {}

  void dump() const {
    llvm::outs() << "CheckInfo: ";
    BaseInfo::dump();
  }

  bool useData(StateInfo& SI) const {
    SI.setD(data);
    return true;
  }
  void write(StateInfo& SI) const {
    useData(SI);
    CheckSpace::writeData(SI);
  }
  void flush(StateInfo& SI) const {
    useData(SI);
    CheckSpace::flushData(SI);
  }
  void pfence(StateInfo& SI) const {
    useData(SI);
    CheckSpace::pfenceData(SI);
  }
  void vfence(StateInfo& SI) const {}
};

class PairInfo : public BaseInfo {
protected:
  const ValueDecl* check;
  PairInfo(const ValueDecl* data_, const ValueDecl* check_)
      : BaseInfo(data_), check(check_) {}

public:
  virtual void dump() const {
    BaseInfo::dump();
    if (check) {
      llvm::outs() << " " << check->getNameAsString();
    }
  }

  virtual bool useData(StateInfo& SI) const {
    SI.setD(data);
    if (((const char*)data) == SI.VarAddr) {
      return true;
    } else if (((const char*)check) == SI.VarAddr) {
      return false;
    } else {
      llvm::report_fatal_error("not data nor check");
      return false;
    }
  }

  // todo make it all pure abstract
  virtual void write(StateInfo& SI) const {}
  virtual void flush(StateInfo& SI) const {}
  virtual void pfence(StateInfo& SI) const {}
  virtual void vfence(StateInfo& SI) const {}
};

class DclInfo : public PairInfo {

public:
  DclInfo(const ValueDecl* data_, const ValueDecl* check_)
      : PairInfo(data_, check_) {}

  void dump() const {
    llvm::outs() << "DclInfo: ";
    PairInfo::dump();
  }

  virtual void write(StateInfo& SI) const {
    if (useData(SI)) {
      DclSpace::writeData(SI);
    } else {
      DclSpace::writeCheck(SI);
    }
  }
  virtual void flush(StateInfo& SI) const {
    if (useData(SI)) {
      DclSpace::flushData(SI);
    }
  }
  virtual void pfence(StateInfo& SI) const {
    useData(SI);
    DclSpace::pfenceData(SI);
  }
};

class SclInfo : public PairInfo {
public:
  SclInfo(const ValueDecl* data_, const ValueDecl* check_)
      : PairInfo(data_, check_) {}

  void dump() const {
    llvm::outs() << "SclInfo: ";
    PairInfo::dump();
  }

  virtual void write(StateInfo& SI) const {
    if (useData(SI)) {
      SclSpace::writeData(SI);
    } else {
      SclSpace::writeCheck(SI);
    }
  }
  virtual void vfence(StateInfo& SI) const {
    useData(SI);
    SclSpace::vfenceData(SI);
  }
  virtual void pfence(StateInfo& SI) const {
    useData(SI);
    SclSpace::vfenceData(SI);
  }
};

class DclDataToMaskInfo : public DclInfo {
  StringRef maskName;

public:
  DclDataToMaskInfo(const ValueDecl* data_, const ValueDecl* check_)
      : DclInfo(data_, check_) {}

  void dump() const {
    llvm::outs() << "DclDataToMaskInfo: ";
    PairInfo::dump();
  }

  virtual void write(StateInfo& SI) const {
    if (useData(SI)) {
      SclSpace::writeData(SI);
    } else {
      // todo check masking
      SclSpace::writeCheck(SI);
    }
  }
};

class SclDataToMaskInfo : public SclInfo {
public:
  SclDataToMaskInfo(const ValueDecl* data_, const ValueDecl* check_)
      : SclInfo(data_, check_) {}

  void dump() const {
    llvm::outs() << "SclDataToMaskInfo: ";
    PairInfo::dump();
  }

  virtual void write(StateInfo& SI) const {
    if (useData(SI)) {
      SclSpace::writeData(SI);
    } else {
      /*
      if (SI.S && usesMask(SI.S, maskName, false)) {
        // write to c(c)
        SI.setMask();
        SclSpace::writeCheck(SI);
      } else {
        // write to c(d)
      }
      */
    }
  }
};

class MaskToValidInfo : public PairInfo {
protected:
  const AnnotateAttr* ann;
  StringRef maskName;
  MaskToValidInfo(const ValueDecl* data_, const ValueDecl* check_,
                  const AnnotateAttr* ann_, StringRef maskName_)
      : PairInfo(data_, check_), ann(ann_), maskName(maskName_) {}

  enum FieldKind { CHUNK_DATA, CHUNK_CHECK, CHECK_CHECK, NONE };

  virtual FieldKind selectField(StateInfo& SI) const {
    if (((const char*)check) == SI.VarAddr) {
      // transitive check
      return FieldKind::CHECK_CHECK;
    } else if (SI.S) {
      // write case
      if (usesMask(SI.S, maskName, false)) {
        // write check
        SI.setMask();
        return FieldKind::CHUNK_CHECK;
      } else {
        // write data
        SI.setMask();
        return FieldKind::CHUNK_DATA;
      }
    } else {
      // flush case
      return FieldKind::CHUNK_DATA;
    }
  }
};

class DclMaskToValidInfo : public MaskToValidInfo {
public:
  DclMaskToValidInfo(const ValueDecl* data_, const ValueDecl* check_,
                     const AnnotateAttr* ann_, StringRef maskName_)
      : MaskToValidInfo(data_, check_, ann_, maskName_) {}

  void dump() const {
    llvm::outs() << "DclMaskToValidInfo: ";
    PairInfo::dump();
  }

  virtual void write(StateInfo& SI) const {
    switch (selectField(SI)) {
    case FieldKind::CHUNK_DATA: {
      SI.setD(data);
      SclSpace::writeData(SI);
      break;
    }
    case FieldKind::CHUNK_CHECK: {
      SI.setD(data);
      SclSpace::writeCheck(SI);
      if (ann) {
        SI.setD(ann);
        DclSpace::writeData(SI);
      }
      break;
    }
    case FieldKind::CHECK_CHECK: {
      SI.setD(ann);
      DclSpace::writeCheck(SI);
      break;
    }
    default: {
      llvm::report_fatal_error("must be either c(d), c(c), check");
      break;
    }
    }
  }

  virtual void flush(StateInfo& SI) const {
    switch (selectField(SI)) {
    case FieldKind::CHUNK_DATA: {
      if (ann) {
        SI.setD(ann);
        DclSpace::flushData(SI);
      }
      break;
    }
    default: {
      // do nothing
      break;
    }
    }
  }

  virtual void vfence(StateInfo& SI) const {
    SI.setD(data);
    SclSpace::vfenceData(SI);
  }

  virtual void pfence(StateInfo& SI) const {
    SI.setD(data);
    SclSpace::vfenceData(SI);
    if (ann) {
      // has transitive validator
      SI.setD(ann);
      DclSpace::pfenceData(SI);
    }
  }
};

class SclMaskToValidInfo : public MaskToValidInfo {
public:
  SclMaskToValidInfo(const ValueDecl* data_, const ValueDecl* check_,
                     const AnnotateAttr* ann_, StringRef maskName_)
      : MaskToValidInfo(data_, check_, ann_, maskName_) {}

  void dump() const {
    llvm::outs() << "SclMaskToValidInfo: ";
    PairInfo::dump();
  }

  virtual void write(StateInfo& SI) const {
    switch (selectField(SI)) {
    case FieldKind::CHUNK_DATA: {
      SI.setD(data);
      SclSpace::writeData(SI);
      break;
    }
    case FieldKind::CHUNK_CHECK: {
      SI.setD(data);
      SclSpace::writeCheck(SI);
      if (ann) {
        SI.setD(ann);
        SclSpace::writeData(SI);
      }
      break;
    }
    case FieldKind::CHECK_CHECK: {
      SI.setD(ann);
      SclSpace::writeCheck(SI);
      break;
    }
    default: {
      llvm::report_fatal_error("must be either c(d), c(c), check");
      break;
    }
    }
  }
  virtual void vfence(StateInfo& SI) const {
    SI.setD(data);
    SclSpace::vfenceData(SI);
    if (ann) {
      // has transitive validator
      SI.setD(ann);
      SclSpace::vfenceData(SI);
    }
  }

  virtual void pfence(StateInfo& SI) const {
    SI.setD(data);
    SclSpace::vfenceData(SI);
    if (ann) {
      // has transitive validator
      SI.setD(ann);
      SclSpace::vfenceData(SI);
    }
  }
};

} // namespace clang::ento::nvm