#pragma once

#include "Common.h"
#include "TransitionInfos.h"
#include "Transitions.h"

namespace clang::ento::nvm {

class BaseInfo {
protected:
  const ValueDecl* data;
  enum Kind { C, DCL, SCL, DCLDM, SCLDM, DCLMV, SCLMV } K;
  BaseInfo(const ValueDecl* data_, Kind K_) : data(data_), K(K_) {}

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
  CheckInfo(const ValueDecl* data_) : BaseInfo(data_, Kind::C) {}

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
  PairInfo(const ValueDecl* data_, Kind K_, const ValueDecl* check_)
      : BaseInfo(data_, K_), check(check_) {}

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
      : PairInfo(data_, Kind::DCL, check_) {}

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
      : PairInfo(data_, Kind::SCL, check_) {}

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

class MaskInfo : public PairInfo {
protected:
  MaskInfo(const ValueDecl* data_, Kind K_, const ValueDecl* check_)
      : PairInfo(data_, K_, check_) {}
};

class DclDataToMaskInfo : public MaskInfo {
public:
  DclDataToMaskInfo(const ValueDecl* data_, const ValueDecl* check_)
      : MaskInfo(data_, Kind::DCLDM, check_) {}

  void dump() const {
    llvm::outs() << "DclDataToMaskInfo: ";
    PairInfo::dump();
  }
};

class SclDataToMaskInfo : public MaskInfo {
public:
  SclDataToMaskInfo(const ValueDecl* data_, const ValueDecl* check_)
      : MaskInfo(data_, Kind::SCLDM, check_) {}

  void dump() const {
    llvm::outs() << "SclDataToMaskInfo: ";
    PairInfo::dump();
  }
};

class MaskToValidInfo : public MaskInfo {
protected:
  const AnnotateAttr* ann;
  MaskToValidInfo(const ValueDecl* data_, Kind K_, const ValueDecl* check_,
                  const AnnotateAttr* ann_)
      : MaskInfo(data_, K_, check_), ann(ann_) {}
};

class DclMaskToValidInfo : public MaskToValidInfo {
public:
  DclMaskToValidInfo(const ValueDecl* data_, const ValueDecl* check_,
                     const AnnotateAttr* ann_)
      : MaskToValidInfo(data_, Kind::DCLMV, check_, ann_) {}

  void dump() const {
    llvm::outs() << "DclMaskToValidInfo: ";
    PairInfo::dump();
  }
};

class SclMaskToValidInfo : public MaskToValidInfo {
public:
  SclMaskToValidInfo(const ValueDecl* data_, const ValueDecl* check_,
                     const AnnotateAttr* ann_)
      : MaskToValidInfo(data_, Kind::SCLMV, check_, ann_) {}

  void dump() const {
    llvm::outs() << "SclMaskToValidInfo: ";
    PairInfo::dump();
  }
};

} // namespace clang::ento::nvm