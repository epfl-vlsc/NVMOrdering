#pragma once
#include "Common.h"

namespace clang::ento::nvm {

void printND(const NamedDecl* ND, const char* msg, bool isQualified = false) {
  llvm::errs() << msg << ":";
  if (!isQualified) {
    llvm::errs() << ND->getNameAsString();
  } else {
    llvm::errs() << ND->getQualifiedNameAsString();
  }

  llvm::errs() << "\n";
}

void printMsg(const char* msg) { llvm::errs() << msg << "\n"; }

void printLoc(const SVal& Loc, const char* msg) {
  llvm::errs() << msg << ":";
  Loc.dump();
  llvm::errs() << "\n";
}

void printStmt(const Stmt* S, CheckerContext& C, const char* msg,
               bool isPretty = false) {
  llvm::errs() << msg << ":";
  if (isPretty) {
    S->dumpPretty(C.getASTContext());
  } else {
    S->dump();
  }
  llvm::errs() << "\n";
}

template <typename MapSet>
void printMapSet(const MapSet& ms, const char* msgPtr, const char* msgElement) {
  for (auto& [ptr, list] : ms) {
    printND(ptr, msgPtr);
    for (auto& element : list) {
      llvm::errs() << msgElement << " " << element->getNameAsString() << "\n";
    }
  }
}

template <typename Set> void printSet(const Set& s, const char* msgElement) {
  for (auto& element : s) {
    llvm::errs() << msgElement << " " << element->getNameAsString() << "\n";
  }
}

} // namespace clang::ento::nvm