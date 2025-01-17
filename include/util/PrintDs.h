#pragma once
#include "Common.h"

namespace clang::ento::nvm {

template <typename Vector> void dumpVector(const Vector& vector) {
  for (auto& e : vector) {
    e.dump();
  }
}

void printReg(const MemRegion* MR, const char* msg) {
  if (MR) {
    llvm::errs() << msg << ":";
    MR->dump();
    llvm::errs() << "\n";
  }
}

void printD(const Decl* D, const char* msg) {
  llvm::errs() << msg << ":";
  D->dump();
  llvm::errs() << "\n";
}

void printND(const NamedDecl* ND, const char* msg, bool isQualified = false,
             bool newline = true) {
  llvm::errs() << msg << ":";
  if (!isQualified)
    llvm::errs() << ND->getNameAsString();
  else
    llvm::errs() << ND->getQualifiedNameAsString();

  if (newline)
    llvm::errs() << "\n";
}

template <typename LatticeValue>
void printTrackedVar(const NamedDecl* ND, const LatticeValue& LV,
                     bool newline = true) {
  llvm::errs() << "tracked:" << ND->getNameAsString();
  LV.dump();
  if (newline)
    llvm::errs() << "\n";
}

void printMsg(const char* msg, bool newline = true) {
  llvm::errs() << msg;
  if (newline)
    llvm::errs() << "\n";
}

void printLoc(const SVal& Loc, const char* msg) {
  llvm::errs() << msg << ":";
  Loc.dump();
  llvm::errs() << "\n";
}

void printStmt(const Stmt* S, AnalysisManager& Mgr, const char* msg,
               bool newline = true) {
  llvm::errs() << msg << ":";
  S->dumpPretty(Mgr.getASTContext());
  if (newline)
    llvm::errs() << "\n";
}

void printStmt(const Stmt* S, const char* msg, bool newline = true) {
  llvm::errs() << msg << ":";
  S->dump();
  if (newline)
    llvm::errs() << "\n";
}

void printBlock(const CFGBlock* block, const char* msg, bool newline=true) {
  // exit is 0, entry is N
  llvm::errs() << msg << ":B" << block->getBlockID();
  if (newline)
    llvm::errs() << "\n";
}

template <typename T> void dumpPtr(const T* t) { t->dump(); }

template <typename T> void dumpObj(const T& t) { t.dump(); }

template <typename MapSet, typename DumpPrint>
void printMapSet(const MapSet& ms, DumpPrint dumpPrint, const char* msgPtr,
                 const char* msgElement) {
  for (auto& [ptr, set] : ms) {
    llvm::errs() << msgPtr << " " << ptr->getQualifiedNameAsString() << " "
                 << set.size() << "\n";
    for (auto& element : set) {
      llvm::errs() << msgElement << " ";
      dumpPrint(element);
      llvm::errs() << "\n";
    }
  }
}

template <typename Set> void printSet(const Set& s, const char* msgElement) {
  for (auto& element : s) {
    llvm::errs() << msgElement << " " << element->getNameAsString() << "\n";
  }
}

void printRegionKind(const MemRegion* MR) {
  if (!MR)
    return;

  if (auto* L1 = MR->getAs<SubRegion>()) {
    printMsg("SubRegion");
    if (auto* L2 = MR->getAs<AllocaRegion>()) {
      printMsg("AllocaRegion");
    } else if (auto* L2 = MR->getAs<SymbolicRegion>()) {
      printMsg("SymbolicRegion");
    } else if (auto* L2 = MR->getAs<TypedRegion>()) {
      printMsg("TypedRegion");
      if (auto* L3 = MR->getAs<TypedValueRegion>()) {
        printMsg("TypedValueRegion");
        if (auto* L4 = MR->getAs<VarRegion>()) {
          printMsg("VarRegion");
        } else if (auto* L4 = MR->getAs<ElementRegion>()) {
          printMsg("ElementRegion");
        }
      }
    }
  } else if (auto* L1 = MR->getAs<MemSpaceRegion>()) {
    printMsg("MemSpaceRegion");
    if (auto* L1 = MR->getAs<HeapSpaceRegion>()) {
      printMsg("HeapSpaceRegion");
    }
  }
}

} // namespace clang::ento::nvm