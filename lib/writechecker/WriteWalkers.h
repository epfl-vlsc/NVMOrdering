#pragma once
#include "Common.h"
#include "identify/OrderFncs.h"
#include "state_machine/OrderVars.h"
#include "walkers/OrderWalker.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace clang::ento::nvm {

class WriteWalker
    : public TUDWalker<WriteWalker, BaseInfo, OrderVars, OrderFncs> {

  void addCheck(const ValueDecl* dataVD) {
    orderVars.addUsedVar(dataVD, new CheckInfo(dataVD));
  }
  BaseInfo* addClMaskToValidInfo(const ValueDecl* dataVD, const ValueDecl* checkVD,
                            const AnnotateAttr* dataAA,
                            const AnnotVarInfo& dataAVI) {
    BaseInfo* BI = nullptr;
    StringRef maskName = dataAVI.getMask();
    if (dataAVI.getClType() == AnnotVarInfo::Dcl) {
      BI = new DclMaskToValidInfo(dataVD, checkVD, dataAA, maskName);
    } else if (dataAVI.getClType() == AnnotVarInfo::Scl) {
      BI = new SclMaskToValidInfo(dataVD, checkVD, dataAA, maskName);
    } else {
      llvm::report_fatal_error("mask to valid info error");
    }

    orderVars.addUsedVar(dataVD, BI);
    if(dataAA){
      orderVars.addUsedVar(dataAA, BI);
    }
    return BI;
  }

  BaseInfo* addClInfo(const ValueDecl* dataVD, const ValueDecl* checkVD,
                 const AnnotVarInfo& dataAVI) {
    BaseInfo* BI = nullptr;
    if (dataAVI.getClType() == AnnotVarInfo::Dcl) {
      BI = new DclInfo(dataVD, checkVD);
    } else if (dataAVI.getClType() == AnnotVarInfo::Scl) {
      BI = new SclInfo(dataVD, checkVD);
    } else {
      llvm::report_fatal_error("mask to valid info error");
    }

    orderVars.addUsedVar(dataVD, BI);
    return BI;
  }

  BaseInfo* addClDataToMaskInfo(const ValueDecl* dataVD, const ValueDecl* checkVD,
                           const AnnotVarInfo& dataAVI) {
    BaseInfo* BI = nullptr;
    if (dataAVI.getClType() == AnnotVarInfo::Dcl) {
      BI = new DclDataToMaskInfo(dataVD, checkVD);
    } else if (dataAVI.getClType() == AnnotVarInfo::Scl) {
      BI = new SclDataToMaskInfo(dataVD, checkVD);
    } else {
      llvm::report_fatal_error("mask to valid info error");
    }

    orderVars.addUsedVar(dataVD, BI);
    return BI;
  }

  void addPair(const ValueDecl* dataVD, const AnnotateAttr* dataAA,
               const AnnotVarInfo& dataAVI) {
    auto checkName = dataAVI.getCheckName();
    const ValueDecl* checkVD = getCheckVD(checkName);
    AnnotVarInfo* checkAVI = getAVI(checkVD);
    BaseInfo* BI = nullptr;

    if (dataAVI.isMask()) {
      // if data is masked
      if (dataVD == checkVD) {
        // pure masked
        BI = addClMaskToValidInfo(dataVD, nullptr, nullptr, dataAVI);
      } else {
        // masked with validator
        BI = addClMaskToValidInfo(dataVD, checkVD, dataAA, dataAVI);
      }
    } else {
      // data is not masked
      if (checkAVI && checkAVI->isMask()) {
        // check uses mask
        BI = addClDataToMaskInfo(dataVD, checkVD, dataAVI);
        llvm::report_fatal_error("mask disable");
      } else {
        // check is not masked
        BI = addClInfo(dataVD, checkVD, dataAVI);
      }
    }

    // also subscribe valids
    if (checkVD && BI) {
      orderVars.addUsedVar(checkVD, BI);
    }
  }

  void addAnnotation(const ValueDecl* dataVD, const AnnotateAttr* dataAA,
                     const AnnotVarInfo& AVI) {

    if (AVI.getClType() == AnnotVarInfo::Check) {
      addCheck(dataVD);
    } else {
      addPair(dataVD, dataAA, AVI);
    }

  } // namespace clang::ento::nvm

public:
  WriteWalker(OrderVars& orderVars_, OrderFncs& orderFncs_)
      : TUDWalker(orderVars_, orderFncs_) {}
};

} // namespace clang::ento::nvm