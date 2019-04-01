#pragma once

#include "Common.h"
#include "Walkers.h"
#include "TransitionInfos.h"
#include "Transitions.h"

namespace clang::ento::nvm {

class BaseInfo {
protected:
  const ValueDecl* data;
  BaseInfo(const ValueDecl* data_) : data(data_) {}

public:
  virtual void dump() const { llvm::outs() << data->getNameAsString(); }

  virtual bool useData(ReportInfos& RI) const = 0;
  virtual void write(ReportInfos& RI) const = 0;
  virtual void flush(ReportInfos& RI) const = 0;
  virtual void pfence(ReportInfos& RI) const = 0;
  virtual void vfence(ReportInfos& RI) const = 0;
};

class CheckInfo : public BaseInfo {

public:
  CheckInfo(const ValueDecl* data_) : BaseInfo(data_) {}

  void dump() const {
    llvm::outs() << "CheckInfo: ";
    BaseInfo::dump();
  }

  bool useData(ReportInfos& RI) const {
    RI.setD(data);
    return true;
  }
  void write(ReportInfos& RI) const {
    useData(RI);
    CheckSpace::writeData(RI);
  }
  void flush(ReportInfos& RI) const {
    useData(RI);
    CheckSpace::flushData(RI);
  }
  void pfence(ReportInfos& RI) const {
    useData(RI);
    CheckSpace::pfenceData(RI);
  }
  void vfence(ReportInfos& RI) const {}
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

  virtual bool useData(ReportInfos& RI) const {
    RI.setD(data);
    if (((const char*)data) == RI.VarAddr) {
      return true;
    } else if (((const char*)check) == RI.VarAddr) {
      return false;
    } else {
      llvm::report_fatal_error("not data nor check");
      return false;
    }
  }

  // todo make it all pure abstract
  virtual void write(ReportInfos& RI) const {}
  virtual void flush(ReportInfos& RI) const {}
  virtual void pfence(ReportInfos& RI) const {}
  virtual void vfence(ReportInfos& RI) const {}
};

class DclInfo : public PairInfo {

public:
  DclInfo(const ValueDecl* data_, const ValueDecl* check_)
      : PairInfo(data_, check_) {}

  void dump() const {
    llvm::outs() << "DclInfo: ";
    PairInfo::dump();
  }

  virtual void write(ReportInfos& RI) const {
    if (useData(RI)) {
      DclSpace::writeData(RI);
    } else {
      DclSpace::writeCheck(RI);
    }
  }
  virtual void flush(ReportInfos& RI) const {
    if (useData(RI)) {
      DclSpace::flushData(RI);
    }
  }
  virtual void pfence(ReportInfos& RI) const {
    useData(RI);
    DclSpace::pfenceData(RI);
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

  virtual void write(ReportInfos& RI) const {
    if (useData(RI)) {
      SclSpace::writeData(RI);
    } else {
      SclSpace::writeCheck(RI);
    }
  }
  virtual void vfence(ReportInfos& RI) const {
    useData(RI);
    SclSpace::vfenceData(RI);
  }
  virtual void pfence(ReportInfos& RI) const {
    useData(RI);
    SclSpace::vfenceData(RI);
  }
};

class DclDataToMaskInfo : public DclInfo {
public:
  DclDataToMaskInfo(const ValueDecl* data_, const ValueDecl* check_)
      : DclInfo(data_, check_) {}

  void dump() const {
    llvm::outs() << "DclDataToMaskInfo: ";
    PairInfo::dump();
  }

  virtual void write(ReportInfos& RI) const {
    if (useData(RI)) {
      SclSpace::writeData(RI);
    } else {
      SclSpace::writeCheck(RI);
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

  virtual void write(ReportInfos& RI) const {
    if (useData(RI)) {
      SclSpace::writeData(RI);
    } else {
      if(RI.S && usesMask(RI.S, false)){
        //write to c(c)
        RI.setMask();
        SclSpace::writeCheck(RI);
      }else{
        //write to c(d)
      }
    }
  }
};

class MaskToValidInfo : public PairInfo {
protected:
  const AnnotateAttr* ann;
  MaskToValidInfo(const ValueDecl* data_, const ValueDecl* check_,
                  const AnnotateAttr* ann_)
      : PairInfo(data_, check_), ann(ann_) {}

  enum FieldKind { CHUNK_DATA, CHUNK_CHECK, CHECK_CHECK, NONE };

  virtual FieldKind selectField(ReportInfos& RI) const {
    if (((const char*)check) == RI.VarAddr) {
      // transitive check
      return FieldKind::CHECK_CHECK;
    } else if (RI.S) {
      // write case
      if (usesMask(RI.S, false)) {
        // write check
        RI.setMask();
        return FieldKind::CHUNK_CHECK;
      } else {
        // write data
        RI.setMask();
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
                     const AnnotateAttr* ann_)
      : MaskToValidInfo(data_, check_, ann_) {}

  void dump() const {
    llvm::outs() << "DclMaskToValidInfo: ";
    PairInfo::dump();
  }

  virtual void write(ReportInfos& RI) const {
    switch (selectField(RI)) {
    case FieldKind::CHUNK_DATA: {
      RI.setD(data);
      SclSpace::writeData(RI);
      break;
    }
    case FieldKind::CHUNK_CHECK: {
      RI.setD(data);
      SclSpace::writeCheck(RI);
      if (ann) {
        RI.setD(ann);
        DclSpace::writeData(RI);
      }
      break;
    }
    case FieldKind::CHECK_CHECK: {
      RI.setD(ann);
      DclSpace::writeCheck(RI);
      break;
    }
    default: {
      llvm::report_fatal_error("must be either c(d), c(c), check");
      break;
    }
    }
  }

  virtual void flush(ReportInfos& RI) const {
    switch (selectField(RI)) {
    case FieldKind::CHUNK_DATA: {
      if (ann) {
        RI.setD(ann);
        DclSpace::flushData(RI);
      }
      break;
    }
    default: {
      //do nothing
      break;
    }
    }
  }

  virtual void vfence(ReportInfos& RI) const {
    RI.setD(data);
    SclSpace::vfenceData(RI);
  }

  virtual void pfence(ReportInfos& RI) const {
    RI.setD(data);
    SclSpace::vfenceData(RI);
    if (ann) {
      // has transitive validator
      RI.setD(ann);
      DclSpace::pfenceData(RI);
    }
  }
};

class SclMaskToValidInfo : public MaskToValidInfo {
public:
  SclMaskToValidInfo(const ValueDecl* data_, const ValueDecl* check_,
                     const AnnotateAttr* ann_)
      : MaskToValidInfo(data_, check_, ann_) {}

  void dump() const {
    llvm::outs() << "SclMaskToValidInfo: ";
    PairInfo::dump();
  }

  virtual void write(ReportInfos& RI) const {
    switch (selectField(RI)) {
    case FieldKind::CHUNK_DATA: {
      RI.setD(data);
      SclSpace::writeData(RI);
      break;
    }
    case FieldKind::CHUNK_CHECK: {
      RI.setD(data);
      SclSpace::writeCheck(RI);
      if (ann) {
        RI.setD(ann);
        SclSpace::writeData(RI);
      }
      break;
    }
    case FieldKind::CHECK_CHECK: {
      RI.setD(ann);
      SclSpace::writeCheck(RI);
      break;
    }
    default: {
      llvm::report_fatal_error("must be either c(d), c(c), check");
      break;
    }
    }
  }
  virtual void vfence(ReportInfos& RI) const {
    RI.setD(data);
    SclSpace::vfenceData(RI);
    if (ann) {
      // has transitive validator
      RI.setD(ann);
      SclSpace::vfenceData(RI);
    }
  }

  virtual void pfence(ReportInfos& RI) const {
    RI.setD(data);
    SclSpace::vfenceData(RI);
    if (ann) {
      // has transitive validator
      RI.setD(ann);
      SclSpace::vfenceData(RI);
    }
  }
};

} // namespace clang::ento::nvm