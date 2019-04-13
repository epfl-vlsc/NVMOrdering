#pragma once
#include "Common.h"
#include "identify/OrderFncs.h"
#include "identify/OrderVars.h"
#include "state_machine/AnnotInfo.h"
#include "walkers/OrderWalker.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace clang::ento::nvm {

using OrderVarsBI = OrderVars<BaseInfo>;

class ReadWalker
    : public TUDWalker<ReadWalker, BaseInfo, OrderVarsBI, OrderFncs> {

  BaseInfo* addClMaskToValidInfo(const ValueDecl* dataVD,
                                 const ValueDecl* checkVD,
                                 const AnnotateAttr* dataAA,
                                 const AnnotVarInfo* dataAVI) {
    BaseInfo* BI = nullptr;
    StringRef maskName = dataAVI->getMask();
    BI = new RecMaskToValidInfo(dataVD, checkVD, dataAA, maskName);
    orderVars.addUsedVar(dataVD, BI);
    if (dataAA) {
      orderVars.addUsedVar(dataAA, BI);
    }
    return BI;
  }

  BaseInfo* addClInfo(const ValueDecl* dataVD, const ValueDecl* checkVD,
                      const AnnotVarInfo* dataAVI) {
    BaseInfo* BI = nullptr;
    BI = new RecInfo(dataVD, checkVD);

    orderVars.addUsedVar(dataVD, BI);
    return BI;
  }

  BaseInfo* addClDataToMaskInfo(const ValueDecl* dataVD,
                                const ValueDecl* checkVD,
                                const AnnotVarInfo* dataAVI,
                                const StringRef& maskName) {
    BaseInfo* BI = nullptr;
    BI = new RecDataToMaskInfo(dataVD, checkVD, maskName);

    orderVars.addUsedVar(dataVD, BI);
    return BI;
  }

  void addPair(const ValueDecl* dataVD, const AnnotateAttr* dataAA,
               const AnnotVarInfo* dataAVI) {
    auto checkName = dataAVI->getCheckName();
    const ValueDecl* checkVD = getCheckVD(checkName);
    AnnotVarInfo* checkAVI = getAVI(checkVD);
    BaseInfo* BI = nullptr;

    if (!dataAVI) {
      llvm::report_fatal_error("data must be tracked for AVI");
    }

    if (dataAVI->isMask()) {
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
        StringRef maskName = checkAVI->getMask();
        // check uses mask
        BI = addClDataToMaskInfo(dataVD, checkVD, dataAVI, maskName);
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
                     const AnnotVarInfo* AVI) {

    if (AVI->getClType() != AnnotVarInfo::Check) {
      addPair(dataVD, dataAA, AVI);
    }
  }

public:
  ReadWalker(OrderVarsBI& orderVars_, OrderFncs& orderFncs_,
             const ASTContext& ASTC_)
      : TUDWalker(orderVars_, orderFncs_, ASTC_) {}
};

} // namespace clang::ento::nvm