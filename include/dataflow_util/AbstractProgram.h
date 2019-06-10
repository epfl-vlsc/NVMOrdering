#pragma once
#include "Common.h"
#include "ProgramLocation.h"

namespace clang::ento::nvm {

class AbstractCfg;

class AbstractBlock {
protected:
  enum BlockType { EntryBlock, ExitBlock, NormalBlock };
  using Stmts = std::vector<const Stmt*>;
  using Blocks = std::vector<AbstractBlock*>;

  // block data
  const CFGBlock* block;
  AbstractCfg& parent;
  size_t blockNo;
  BlockType blockType;

  // structures
  Stmts stmts;
  Blocks succs;
  Blocks preds;

  BlockType getBlockType(const CFGBlock* block) {
    CFG* cfg = block->getParent();
    CFGBlock& entryBlock = cfg->back();
    CFGBlock& exitBlock = cfg->front();

    if (&entryBlock == block) {
      return EntryBlock;
    } else if (&exitBlock == block) {
      return ExitBlock;
    } else {
      return NormalBlock;
    }
  }

public:
  // normal constructor
  AbstractBlock(const CFGBlock* block_, AbstractCfg& parent_)
      : block(block_), parent(parent_) {
    blockType = getBlockType(block_);
    blockNo = getBlockId(block_);
  }

  static size_t getBlockId(const CFGBlock* block_) {
    const CFG* cfg = block_->getParent();
    size_t numBlocks = cfg->size();
    return numBlocks - block_->getBlockID() - 1;
  }

  AbstractCfg& getParent() { return parent; }

  bool isEntryExit() const { return blockType != NormalBlock; }

  void insertStmt(const Stmt* S) {
    if (isEntryExit()) {
      llvm::report_fatal_error("disable adding stmt to entry exit block");
    }
    stmts.push_back(S);
  }

  void insertSucc(AbstractBlock& block_) { succs.push_back(&block_); }

  void insertPred(AbstractBlock& block_) { preds.push_back(&block_); }

  auto getCFGElements() const {
    return llvm::iterator_range(block->begin(), block->end());
  }

  auto getStmts() const {
    return llvm::iterator_range(stmts.begin(), stmts.end());
  }

  auto getCFGSuccessors() const {
    return llvm::iterator_range(block->succ_begin(), block->succ_end());
  }

  auto getSuccessors() const {
    return llvm::iterator_range(succs.begin(), succs.end());
  }

  auto getCFGPredecessors() const {
    return llvm::iterator_range(block->pred_begin(), block->pred_end());
  }

  auto getPredecessors() const {
    return llvm::iterator_range(preds.begin(), preds.end());
  }

  const Stmt* getEntryStmt() { return stmts[0]; }

  const Stmt* getExitStmt() { return stmts[numStmts() - 1]; }

  size_t numStmts() const { return stmts.size(); }

  void dump() const { llvm::errs() << "B" << blockNo; }

  void dump(AnalysisManager* Mgr) const {
    switch (blockType) {
    case EntryBlock:
      llvm::errs() << "entry ";
      break;
    case ExitBlock:
      llvm::errs() << "exit ";
      break;
    default:
      break;
    }

    llvm::errs() << "B" << blockNo << "\n";
    for (auto stmt : stmts) {
      printStmt(stmt, *Mgr, "\ts");
    }

    printMsg("successors:", false);
    for (auto succ : succs) {
      succ->dump();
      llvm::errs() << " ";
    }
    llvm::errs() << "\n";

    printMsg("predecessors:", false);
    for (auto pred : preds) {
      pred->dump();
      llvm::errs() << " ";
    }
    llvm::errs() << "\n";
  }
};

class AbstractCfg {
  using Blocks = std::vector<AbstractBlock>;

  const FunctionDecl* function;
  Blocks blocks;

public:
  AbstractBlock& insertBlock(const CFGBlock* block) {
    blocks.emplace_back(block, *this);
    return blocks.back();
  }

  auto getCFGBlocks(AnalysisManager* Mgr) const {
    const CFG* cfg = Mgr->getCFG(function);
    return llvm::iterator_range(cfg->rbegin(), cfg->rend());
  }

  auto getBlocks() {
    return llvm::iterator_range(blocks.begin(), blocks.end());
  }

  auto begin() const { return blocks.begin(); }

  auto end() const { return blocks.end(); }

  AbstractBlock& getBlock(size_t no) { return blocks[no]; }

  AbstractBlock& getEntryBlock() { return blocks.front(); }

  AbstractBlock& getExitBlock() { return blocks.back(); }

  size_t numBlocks() const { return blocks.size(); }

  // normal constructor
  AbstractCfg() {}

  void setFunction(const FunctionDecl* function_) { function = function_; }

  void dump(AnalysisManager* Mgr) const {
    printND(function, "*****function*****");
    for (auto block : blocks) {
      block.dump(Mgr);
    }
  }
};

class AbstractProgram {
  using FunctionCfgMap = std::map<const FunctionDecl*, AbstractCfg>;

  FunctionCfgMap functionCfgMap;

public:
  AbstractCfg& insertAbstractCfg(const FunctionDecl* function) {
    auto& abstractCfg = functionCfgMap[function];
    abstractCfg.setFunction(function);
    return abstractCfg;
  }

  AbstractCfg& getFunctionGraph(const FunctionDecl* function) {
    assert(functionCfgMap.count(function));
    return functionCfgMap[function];
  }

  size_t numFunctions() const { return functionCfgMap.size(); }

  AbstractProgram() {}

  void dump(AnalysisManager* Mgr) const {
    llvm::errs() << "Num functions: " << numFunctions() << "\n";
    for (auto& [_, abstractCfg] : functionCfgMap) {
      abstractCfg.dump(Mgr);
    }
  }
};

template <typename Transitions> class AbstractProgramBuilder {
  // useful structures
  Transitions& transitions;
  AnalysisManager* Mgr;

  void buildAbstractBlock(AbstractBlock& abstractBlock) {
    // skip if entry or exit block
    if (abstractBlock.isEntryExit()) {
      return;
    }

    // iterate blocks
    for (const CFGElement element : abstractBlock.getCFGElements()) {
      if (Optional<CFGStmt> CS = element.getAs<CFGStmt>()) {
        const Stmt* S = CS->getStmt();
        assert(S);

        // check if stmt is used in data flow
        auto PTI = transitions.parseStmt(S);
        if (PTI.isStmtUsed()) {
          abstractBlock.insertStmt(S);
        }
      }
    }
  }

  void buildSuccPred(AbstractCfg& parent, AbstractBlock& abstractBlock) {
    for (const CFGBlock* predBlock : abstractBlock.getCFGPredecessors()) {
      size_t no = AbstractBlock::getBlockId(predBlock);
      auto& predAbstractBlock = parent.getBlock(no);
      abstractBlock.insertPred(predAbstractBlock);
    }

    for (const CFGBlock* succBlock : abstractBlock.getCFGSuccessors()) {
      size_t no = AbstractBlock::getBlockId(succBlock);
      auto& succAbstractBlock = parent.getBlock(no);
      abstractBlock.insertSucc(succAbstractBlock);
    }
  }

  void buildAbstractCfg(AbstractCfg& abstractCfg) {
    // per function abstract cfg

    // iterate cfg
    for (const CFGBlock* block : abstractCfg.getCFGBlocks(Mgr)) {
      // per block abstract block
      auto& abstractBlock = abstractCfg.insertBlock(block);
      buildAbstractBlock(abstractBlock);
    }

    for (auto& abstractBlock : abstractCfg.getBlocks()) {
      // nuild succ and pred
      buildSuccPred(abstractCfg, abstractBlock);
    }
  }

public:
  AbstractProgramBuilder(Transitions& transitions_)
      : transitions(transitions_) {
    Mgr = transitions.getMgr();
    // set so that all effected statements are tracked, not just unit
    transitions.useGlobalTransitions();
    assert(Mgr);
  }

  void buildProgram(AbstractProgram& abstractProgram) {
    // create abstract control flow graphs for each function
    for (const FunctionDecl* function : transitions.getAnalysisFunctions()) {
      auto& abstractCfg = abstractProgram.insertAbstractCfg(function);
      buildAbstractCfg(abstractCfg);
    }
  }
};

} // namespace clang::ento::nvm
