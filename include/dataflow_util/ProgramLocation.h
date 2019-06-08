#pragma once
#include "Common.h"

namespace clang::ento::nvm {

struct ProgramLocation {
  union PtrPl {
    const FunctionDecl* function;
    const CFGBlock* block;
    const Stmt* stmt;

    PtrPl(const FunctionDecl* pl) {
      assert(pl != nullptr);
      function = pl;
    }
    PtrPl(const CFGBlock* pl) {
      assert(pl != nullptr);
      block = pl;
    }
    PtrPl(const Stmt* pl) {
      assert(pl != nullptr);
      stmt = pl;
    }
    PtrPl() { function = nullptr; }
  } ptrPl;
  enum PtrType { CfgPtr, CfgBlockPtr, StmtPtr, None } ptrType;

  ProgramLocation(const FunctionDecl* pl) : ptrPl(pl), ptrType(CfgPtr) {}
  ProgramLocation(const CFGBlock* pl) : ptrPl(pl), ptrType(CfgBlockPtr) {}
  ProgramLocation(const Stmt* pl) : ptrPl(pl), ptrType(StmtPtr) {}
  ProgramLocation() : ptrType(None) {}

  bool operator<(const ProgramLocation& X) const {
    return ptrPl.function < X.ptrPl.function;
  }

  bool operator==(const ProgramLocation& X) const {
    return ptrPl.function == X.ptrPl.function;
  }

  void dump() const {
    switch (ptrType) {
    case CfgPtr:
      printND(ptrPl.function, "function");
      break;
    case CfgBlockPtr:
      printBlock(ptrPl.block, "block");
      break;
    case StmtPtr:
      printStmt(ptrPl.stmt, "stmt");
      break;
    case None:
      printMsg("None");
      break;
    default:
      llvm::report_fatal_error("not possible location");
      break;
    }
  }

  void dump(AnalysisManager& mgr) const {
    switch (ptrType) {
    case StmtPtr: {
      printStmt(ptrPl.stmt, mgr, "stmt");
      return;
    } break;
    default:
      dump();
      break;
    }
  }

  const FunctionDecl* getFunction() const { return ptrPl.function; }
  const CFGBlock* getBlock() const { return ptrPl.block; }
  const Stmt* getStmt() const { return ptrPl.stmt; }
};

class PlContext {
  ProgramLocation caller;
  ProgramLocation callee;

public:
  PlContext() {}
  PlContext(const PlContext& X, const ProgramLocation& Callee) {
    caller = X.callee;
    callee = Callee;
  }

  bool operator<(const PlContext& X) const {
    return (caller < X.caller && callee < X.callee);
  }

  bool operator==(const PlContext& X) const {
    return (caller == X.caller && callee == X.callee);
    ;
  }

  void dump() const {
    llvm::errs() << "caller: ";
    caller.dump();
    llvm::errs() << "callee: ";
    callee.dump();
  }

  void dump(AnalysisManager* mgr) const {
    llvm::errs() << "caller: ";
    caller.dump(*mgr);
    llvm::errs() << "callee: ";
    callee.dump(*mgr);
  }
};

class Forward {
public:
  static auto getElements(const CFGBlock* block) {
    return llvm::iterator_range(block->begin(), block->end());
  }

  static auto getBlocks(const CFG* cfg) {
    return llvm::iterator_range(cfg->begin(), cfg->end());
  }

  static auto getRBlocks(const CFG* cfg) {
    return llvm::iterator_range(cfg->rbegin(), cfg->rend());
  }

  static auto getSuccessorBlocks(const CFGBlock* block) {
    return llvm::iterator_range(block->succ_begin(), block->succ_end());
  }

  static auto getPredecessorBlocks(const CFGBlock* block) {
    return llvm::iterator_range(block->pred_begin(), block->pred_end());
  }

  static auto getPredecessorBlocks(const ProgramLocation& blockLoc) {
    const CFGBlock* block = blockLoc.getBlock();
    return getPredecessorBlocks(block);
  }

  static ProgramLocation getEntryBlock(const CFG* cfg) {
    const CFGBlock* entryBlock = &cfg->getEntry();
    return ProgramLocation(entryBlock);
  }

  static ProgramLocation getStmtKey(const Stmt* S) {
    return ProgramLocation(S);
  }

  static ProgramLocation getFunctionEntryKey(const FunctionDecl* function) {
    return ProgramLocation(function);
  }

  static ProgramLocation getFunctionExitKey(const FunctionDecl* function,
                                         AnalysisManager* mgr) {
    const CFG* cfg = mgr->getCFG(function);
    const CFGBlock* exitBlock = &cfg->getExit();
    return ProgramLocation(exitBlock);
  }

  static ProgramLocation getEntryKey(const CFGBlock* block) {
    return ProgramLocation(block);
  }

  static bool isEntryExit(const CFGBlock* block) {
    CFG* cfg = block->getParent();
    CFGBlock& entryBlock = cfg->front();
    CFGBlock& exitBlock = cfg->back();
    return (&entryBlock == block) || (&exitBlock == block);
  }

  static ProgramLocation getExitKey(const CFGBlock* block) {
    if (isEntryExit(block)) {
      return ProgramLocation(block);
    }

    // process final element in the block
    const CFGElement& element = block->back();
    int kind = element.getKind();

    switch (kind) {
    case CFGElement::Statement: {
      CFGStmt CS = element.castAs<CFGStmt>();
      const Stmt* stmt = CS.getStmt();
      assert(stmt);
      return ProgramLocation(stmt);
    } break;
    default: {
      llvm::errs() << "kind:" << kind << "\n";
      block->dump();
      llvm::report_fatal_error("kind not covered");

      return ProgramLocation();
    } break;
    }
  }
};

} // namespace clang::ento::nvm