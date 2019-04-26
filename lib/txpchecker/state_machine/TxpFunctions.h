#pragma once
#include "Common.h"
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

  bool inAlloc(const IdentifierInfo* II) const {
    return II->isStr("pmalloc") || II->isStr("pmemobj_tx_alloc") ||
           II->isStr("pmemobj_tx_zalloc") || II->isStr("pmemobj_tx_xalloc") ||
           II->isStr("pmemobj_tx_realloc") ||
           II->isStr("pmemobj_tx_zrealloc") || II->isStr("pmemobj_tx_strdup") ||
           II->isStr("pmemobj_tx_wcsdup");
  }

  bool inFree(const IdentifierInfo* II) const {
    return II->isStr("pfree") || II->isStr("pmemobj_tx_free");
  }

  bool inDirect(const IdentifierInfo* II) const {
    return II->isStr("pmemobj_direct");
  }

  bool inAddRange(const IdentifierInfo* II) const {
    return II->isStr("pmemobj_tx_add_range");
  }

  bool inAddRangeDirect(const IdentifierInfo* II) const {
    return II->isStr("pmemobj_tx_add_range_direct");
  }

  bool inTxBeg(const IdentifierInfo* II) const {
    return II->isStr("tx_begin") || II->isStr("pmemobj_tx_begin");
  }

  bool inTxEnd(const IdentifierInfo* II) const {
    return II->isStr("tx_end") || II->isStr("pmemobj_tx_end");
  }

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
      } else {
        // different type of annotation
        return;
      }
    } else {
      // no identifier
      return;
    }
  }

  bool isPFunction(const FunctionDecl* FD) {
    return isPalloc(FD) || isPfree(FD) || isPdirect(FD);
  }

  bool isAnnotatedFnc(const FunctionDecl* FD) { return isTxFnc(FD); }

  bool isTxRangeDirect(const FunctionDecl* FD) {
    return txRangeDirectSet.count(FD);
  }

  bool isTxFnc(const FunctionDecl* FD) { return txFnc.inFunctions(FD); }

  bool isTxRange(const FunctionDecl* FD) { return txRangeSet.count(FD); }

  bool isTxBeg(const FunctionDecl* FD) { return txBegSet.count(FD); }

  bool isTxEnd(const FunctionDecl* FD) { return txEndSet.count(FD); }

  bool isPalloc(const FunctionDecl* FD) { return pallocFncSet.count(FD); }

  bool isPfree(const FunctionDecl* FD) { return pfreeFncSet.count(FD); }

  bool isPdirect(const FunctionDecl* FD) { return pdirectFncSet.count(FD); }

  void dump() {
    txFnc.dump();
    std::set<const FunctionDecl*>* functionSets[] = {
        &txBegSet, &txRangeSet,   &txRangeDirectSet,
        &txEndSet, &pallocFncSet, &pfreeFncSet};
    for (auto* fncSet : functionSets) {
      for (const FunctionDecl* FD : *fncSet) {
        llvm::outs() << FD->getQualifiedNameAsString() << "\n";
      }
    }
  }
};

} // namespace clang::ento::nvm