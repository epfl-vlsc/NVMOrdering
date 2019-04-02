#pragma once

#include "Common.h"
#include "Walkers.h"
#include "TransitionInfos.h"
#include "RecTransitions.h"

namespace clang::ento::nvm {

class BaseInfo {
protected:
  const ValueDecl* data;
  BaseInfo(const ValueDecl* data_) : data(data_) {}

public:
  virtual void dump() const { llvm::outs() << data->getNameAsString(); }

  virtual bool useData(ReportInfos& RI) const = 0;
  virtual void read(ReportInfos& RI) const = 0;
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
  virtual void read(ReportInfos& RI) const{}
};

class RecInfo : public PairInfo {

public:
  RecInfo(const ValueDecl* data_, const ValueDecl* check_)
      : PairInfo(data_, check_) {}

  void dump() const {
    llvm::outs() << "RecInfo: ";
    PairInfo::dump();
  }

  virtual void read(ReportInfos& RI) const {
    if (useData(RI)) {
      RecSpace::readData(RI);
    } else {
      RecSpace::readCheck(RI);
    }
  }
};

class RecDataToMaskInfo : public RecInfo {
public:
  RecDataToMaskInfo(const ValueDecl* data_, const ValueDecl* check_)
      : RecInfo(data_, check_) {}

  void dump() const {
    llvm::outs() << "RecDataToMaskInfo: ";
    PairInfo::dump();
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
      // read case
      if (usesMask(RI.S, true)) {
        // read check
        RI.setMask();
        return FieldKind::CHUNK_CHECK;
      } else {
        // read data
        RI.setMask();
        return FieldKind::CHUNK_DATA;
      }
    } else {
      // flush case
      return FieldKind::CHUNK_DATA;
    }
  }
};

class RecMaskToValidInfo : public MaskToValidInfo {
public:
  RecMaskToValidInfo(const ValueDecl* data_, const ValueDecl* check_,
                     const AnnotateAttr* ann_)
      : MaskToValidInfo(data_, check_, ann_) {}

  void dump() const {
    llvm::outs() << "DclMaskToValidInfo: ";
    PairInfo::dump();
  }

  virtual void read(ReportInfos& RI) const {
    switch (selectField(RI)) {
    case FieldKind::CHUNK_DATA: {
      DBG("chunk data " << (void*)data << "readData")
      RI.setD(data);
      RecSpace::readData(RI);
      break;
    }
    case FieldKind::CHUNK_CHECK: {
      DBG("chunk check " << (void*)data << "readCheck")
      RI.setD(data);
      RecSpace::readCheck(RI);
      if (ann) {
        DBG("chunk check " << (void*)ann << "readData")
        RI.setD(ann);
        RecSpace::readData(RI);
      }
      break;
    }
    case FieldKind::CHECK_CHECK: {
      DBG("check " << (void*)ann << "readCheck")
      RI.setD(ann);
      RecSpace::readCheck(RI);
      break;
    }
    default: {
      llvm::report_fatal_error("must be either c(d), c(c), check");
      break;
    }
    }
  }
};


} // namespace clang::ento::nvm