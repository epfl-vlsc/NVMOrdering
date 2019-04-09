#pragma once
#include "Common.h"

namespace clang::ento::nvm {

class NVMTransactionInfo {
  std::set<const FunctionDecl*> txBegSet;
  std::set<const FunctionDecl*> txRangeSet;
  std::set<const FunctionDecl*> txRangeDirectSet;
  std::set<const FunctionDecl*> txEndSet;

  std::set<const FunctionDecl*> pallocFncSet;
  std::set<const FunctionDecl*> pfreeFncSet;
  std::set<const FunctionDecl*> paccFncSet;

public:
  void insertFunction(const FunctionDecl* FD) {
    const IdentifierInfo* II = FD->getIdentifier();
    
    //todo 
    /*
    II->isStr("pmemobj_tx_alloc") ||
          II->isStr("pmemobj_tx_zalloc") || II->isStr("pmemobj_tx_realloc") ||
          II->isStr("pmemobj_tx_zrealloc")
          */
    if (II) {
      if (II->isStr("pmalloc") || II->isStr("pmemobj_tx_zalloc")) {
        pallocFncSet.insert(FD);
      } else if (II->isStr("pfree") || II->isStr("pmemobj_tx_free")) {
        pfreeFncSet.insert(FD);
      } else if (II->isStr("pmemobj_direct")) {
        paccFncSet.insert(FD);
      } else if (II->isStr("pmemobj_tx_add_range_direct")) {
        txRangeDirectSet.insert(FD);
      } else if (II->isStr("pmemobj_tx_add_range")) {
        txRangeSet.insert(FD);
      } else if (II->isStr("tx_begin") || II->isStr("pmemobj_tx_begin")) {
        txBegSet.insert(FD);
      } else if (II->isStr("tx_end") || II->isStr("pmemobj_tx_end") ||
                 II->isStr("pmemobj_tx_abort")) {
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

  bool isPFunction(CheckerContext& C) {
    const FunctionDecl* FD = getFuncDecl(C);
    return isPalloc(FD) || isPfree(FD) || isPacc(FD);
  }

  bool isTxRangeDirect(const FunctionDecl* FD) {
    return txRangeDirectSet.count(FD);
  }

  bool isTxRange(const FunctionDecl* FD) { return txRangeSet.count(FD); }

  bool isTxBeg(const FunctionDecl* FD) { return txBegSet.count(FD); }

  bool isTxEnd(const FunctionDecl* FD) { return txEndSet.count(FD); }

  bool isPalloc(const FunctionDecl* FD) { return pallocFncSet.count(FD); }

  bool isPfree(const FunctionDecl* FD) { return pfreeFncSet.count(FD); }

  bool isPacc(const FunctionDecl* FD) { return paccFncSet.count(FD); }

  void dumpFunctions() {

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