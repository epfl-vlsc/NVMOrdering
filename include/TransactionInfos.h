#pragma once
#include "Common.h"
#include <set>

namespace clang::ento::nvm {

class NVMTransactionInfo {
  static constexpr const char* TXBEG = "TX_BEG";
  static constexpr const char* TXEND = "TX_END";

  std::set<const FunctionDecl*> txBegSet;
  std::set<const FunctionDecl*> txEndSet;

  std::set<const FunctionDecl*> pallocFncSet;
  std::set<const FunctionDecl*> pfreeFncSet;

public:
  void insertFunction(const FunctionDecl* FD) {
    const IdentifierInfo* II = FD->getIdentifier();
    if (II) {
      if (II->isStr("pmalloc")) {
        pallocFncSet.insert(FD);
      } else if (II->isStr("pfree")) {
        pfreeFncSet.insert(FD);
      } else if (II->isStr("tx_begin")) {
        txBegSet.insert(FD);
      } else if (II->isStr("tx_end")) {
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
    return pallocFncSet.count(FD) || pfreeFncSet.count(FD);
  }

  bool isTxBeg(const FunctionDecl* FD) {
    return txBegSet.count(FD);
  }

  bool isTxEnd(const FunctionDecl* FD) {
    return txEndSet.count(FD);
  }

  void dumpFunctions() {
    std::set<const FunctionDecl*>* functionSets[] = {
        &txBegSet, &txEndSet, &pallocFncSet, &pfreeFncSet};
    for (auto* fncSet : functionSets) {
      for (const FunctionDecl* FD : *fncSet) {
        llvm::outs() << FD->getQualifiedNameAsString() << "\n";
      }
    }
  }
};

} // namespace clang::ento::nvm