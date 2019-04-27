#pragma once
#include "Common.h"
#include "FncNames.h"
#include "identify/AnnotFunction.h"

namespace clang::ento::nvm {

class TxpFunctions {
  AnnotFunction txFnc;
  std::set<const FunctionDecl*> txBegSet;
  std::set<const FunctionDecl*> txRangeSet;
  std::set<const FunctionDecl*> txRangeDirectSet;
  std::set<const FunctionDecl*> txEndSet;

  std::set<const FunctionDecl*> pallocFncSet;
  std::set<const FunctionDecl*> pfreeFncSet;
  std::set<const FunctionDecl*> pdirectFncSet;

  std::set<const FunctionDecl*> pmiscFncSet;

public:
  TxpFunctions() : txFnc("TxCode") {}

  void insertIfKnown(const FunctionDecl* FD) {
    txFnc.insertIfKnown(FD);

    const IdentifierInfo* II = FD->getIdentifier();

    if (II) {
      if (inAlloc(II)) {
        pallocFncSet.insert(FD);
      } else if (inFree(II)) {
        pfreeFncSet.insert(FD);
      } else if (inDirect(II)) {
        pdirectFncSet.insert(FD);
      } else if (inAddRange(II)) {
        txRangeSet.insert(FD);
      } else if (inAddRangeDirect(II)) {
        txRangeDirectSet.insert(FD);
      } else if (inTxBeg(II)) {
        txBegSet.insert(FD);
      } else if (inTxEnd(II)) {
        txEndSet.insert(FD);
      } else if (inMisc(II)) {
        pmiscFncSet.insert(FD);
      } else {
        // different type of annotation
        return;
      }
    } else {
      // no identifier
      return;
    }
  }

  bool isAnyPfnc(const FunctionDecl* FD) const {
    return isPinit(FD) || isPtx(FD) || isPmisc(FD);
  }

  bool isPinit(const FunctionDecl* FD) const {
    return isPalloc(FD) || isPfree(FD) || isPdirect(FD);
  }

  bool isPtx(const FunctionDecl* FD) const {
    return isTxRange(FD) || isTxRangeDirect(FD) || isTxBeg(FD) || isTxEnd(FD);
  }

  bool isAnnotatedFnc(const FunctionDecl* FD) const { return isTxFnc(FD); }

  bool isTxFnc(const FunctionDecl* FD) const { return txFnc.inFunctions(FD); }

  bool isTxRange(const FunctionDecl* FD) const { return txRangeSet.count(FD); }

  bool isTxRangeDirect(const FunctionDecl* FD) const {
    return txRangeDirectSet.count(FD);
  }

  bool isTxBeg(const FunctionDecl* FD) const { return txBegSet.count(FD); }

  bool isTxEnd(const FunctionDecl* FD) const { return txEndSet.count(FD); }

  bool isPalloc(const FunctionDecl* FD) const { return pallocFncSet.count(FD); }

  bool isPfree(const FunctionDecl* FD) const { return pfreeFncSet.count(FD); }

  bool isPdirect(const FunctionDecl* FD) const {
    return pdirectFncSet.count(FD);
  }

  bool isPmisc(const FunctionDecl* FD) const {
    return pmiscFncSet.count(FD);
  }

  void dump() {
    txFnc.dump();
    std::set<const FunctionDecl*>* functionSets[] = {
        &txBegSet, &txRangeSet,   &txRangeDirectSet,
        &txEndSet, &pallocFncSet, &pfreeFncSet, &pmiscFncSet};
    for (auto* fncSet : functionSets) {
      for (const FunctionDecl* FD : *fncSet) {
        llvm::outs() << FD->getQualifiedNameAsString() << "\n";
      }
    }
  }
};

} // namespace clang::ento::nvm