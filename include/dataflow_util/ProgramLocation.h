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
  enum PtrType { FunctionPtr, CfgBlockPtr, StmtPtr, None } ptrType;

  ProgramLocation(const FunctionDecl* pl) : ptrPl(pl), ptrType(FunctionPtr) {}
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
    case FunctionPtr:
      printND(ptrPl.function, "function", true, false);
      break;
    case CfgBlockPtr:
      printBlock(ptrPl.block, "block", false);
      break;
    case StmtPtr:
      printStmt(ptrPl.stmt, "stmt", false);
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
      printStmt(ptrPl.stmt, mgr, "stmt", false);
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
  const CallExpr* caller;
  const CallExpr* callee;

public:
  PlContext() : caller(nullptr), callee(nullptr) {}
  PlContext(const PlContext& X, const CallExpr* Callee) {
    caller = X.callee;
    callee = Callee;
  }

  bool operator<(const PlContext& X) const {
    return (caller < X.caller || callee < X.callee);
  }

  bool operator==(const PlContext& X) const {
    return (caller == X.caller && callee == X.callee);
  }

  bool operator!=(const PlContext& X) const {
    return (caller != X.caller || callee != X.callee);
  }

  void dump() const {
    printMsg("context:", false);
    if (caller)
      printStmt(caller, " caller:", false);
    else
      printMsg(" none", false);

    if (callee)
      printStmt(callee, " callee:", false);
    else
      printMsg(" none", false);
    printMsg("");
  }

  void dump(AnalysisManager& Mgr) const {
    printMsg("context:", false);
    if (caller)
      printStmt(caller, Mgr, " caller:", false);
    else
      printMsg(" none", false);

    if (callee)
      printStmt(callee, Mgr, " callee:", false);
    else
      printMsg(" none", false);

    printMsg("");
  }
};

class Forward {
public:
  static auto getElements(const CFGBlock* block) {
    return llvm::iterator_range(block->begin(), block->end());
  }

  static auto getBlocks(const FunctionDecl* function, AnalysisManager& Mgr) {
    const CFG* cfg = Mgr.getCFG(function);
    return llvm::iterator_range(cfg->begin(), cfg->end());
  }

  static auto getRBlocks(const FunctionDecl* function, AnalysisManager& Mgr) {
    const CFG* cfg = Mgr.getCFG(function);
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

  static ProgramLocation getEntryBlock(const FunctionDecl* function,
                                       AnalysisManager& Mgr) {
    const CFG* cfg = Mgr.getCFG(function);
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
                                            AnalysisManager& Mgr) {
    const CFG* cfg = Mgr.getCFG(function);
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