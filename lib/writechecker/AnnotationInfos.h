#pragma once

#include "Common.h"
#include "StateMachine/Transitions.h"

namespace clang::ento::nvm {

class BaseInfo {
protected:
  const ValueDecl* data;
  enum Kind { C, DCL, SCL, DCLDM, SCLDM, DCLMV, SCLMV } K;
  BaseInfo(const ValueDecl* data_, Kind K_) : data(data_), K(K_) {}

public:
  virtual void dump() { llvm::outs() << data->getNameAsString(); }
  virtual void write(SVal Loc, const Stmt* S, CheckerContext& C,
                     const ValueDecl* VD, ExplodedNode*& EN) = 0;
  virtual void flush(CheckerContext& C, const ValueDecl* VD,
                     ExplodedNode*& EN) = 0;
  virtual void pfence(CheckerContext& C, const ValueDecl* VD,
                      ExplodedNode*& EN) = 0;
  virtual void vfence(CheckerContext& C, const ValueDecl* VD,
                      ExplodedNode*& EN) = 0;
};

class CheckInfo : public BaseInfo {

public:
  CheckInfo(const ValueDecl* data_) : BaseInfo(data_, Kind::C) {}

  void dump() {
    llvm::outs() << "CheckInfo: ";
    BaseInfo::dump();
  }

  void write(SVal Loc, const Stmt* S, CheckerContext& C, const ValueDecl* VD,
             ExplodedNode*& EN) {
    ProgramStateRef State = C.getState();
    CheckSpace::writeData(State, (char*)VD);
    //llvm::outs() << State->get<SclMap>((char*)VD)->getStateName() << "\n";
  }
  void flush(CheckerContext& C, const ValueDecl* VD, ExplodedNode*& EN) {
    // ProgramStateRef State = C.getState();
  }
  void pfence(CheckerContext& C, const ValueDecl* VD, ExplodedNode*& EN) {
    // ProgramStateRef State = C.getState();
  }
  void vfence(CheckerContext& C, const ValueDecl* VD, ExplodedNode*& EN) {}
};

class PairInfo : public BaseInfo {
protected:
  const ValueDecl* check;
  PairInfo(const ValueDecl* data_, Kind K_, const ValueDecl* check_)
      : BaseInfo(data_, K_), check(check_) {}

public:
  virtual void dump() {
    BaseInfo::dump();
    if (check) {
      llvm::outs() << " " << check->getNameAsString();
    }
  }

  virtual void write(SVal Loc, const Stmt* S, CheckerContext& C,
                     const ValueDecl* VD, ExplodedNode*& EN) {
    // ProgramStateRef State = C.getState();
  }
  virtual void flush(CheckerContext& C, const ValueDecl* VD,
                     ExplodedNode*& EN) {
    // ProgramStateRef State = C.getState();
  }
  virtual void pfence(CheckerContext& C, const ValueDecl* VD,
                      ExplodedNode*& EN) {
    // ProgramStateRef State = C.getState();
  }
  virtual void vfence(CheckerContext& C, const ValueDecl* VD,
                      ExplodedNode*& EN) {}
};

class DclInfo : public PairInfo {

public:
  DclInfo(const ValueDecl* data_, const ValueDecl* check_)
      : PairInfo(data_, Kind::DCL, check_) {}

  void dump() {
    llvm::outs() << "DclInfo: ";
    PairInfo::dump();
  }
};

class SclInfo : public PairInfo {
public:
  SclInfo(const ValueDecl* data_, const ValueDecl* check_)
      : PairInfo(data_, Kind::SCL, check_) {}

  void dump() {
    llvm::outs() << "SclInfo: ";
    PairInfo::dump();
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

  void dump() {
    llvm::outs() << "DclDataToMaskInfo: ";
    PairInfo::dump();
  }
};

class SclDataToMaskInfo : public MaskInfo {
public:
  SclDataToMaskInfo(const ValueDecl* data_, const ValueDecl* check_)
      : MaskInfo(data_, Kind::SCLDM, check_) {}

  void dump() {
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

  void dump() {
    llvm::outs() << "DclMaskToValidInfo: ";
    PairInfo::dump();
  }
};

class SclMaskToValidInfo : public MaskToValidInfo {
public:
  SclMaskToValidInfo(const ValueDecl* data_, const ValueDecl* check_,
                     const AnnotateAttr* ann_)
      : MaskToValidInfo(data_, Kind::SCLMV, check_, ann_) {}

  void dump() {
    llvm::outs() << "SclMaskToValidInfo: ";
    PairInfo::dump();
  }
};

} // namespace clang::ento::nvm