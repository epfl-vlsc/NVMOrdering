#pragma once
#include "Common.h"
#include "ProgramLocation.h"

namespace clang::ento::nvm {

class AbstractStmt;
class AbstractBlock;
class AbstractFunction;

struct AbstractLocation {
protected:
  enum LocationType {
    FunctionLocation,
    BlockLocation,
    StmtLocation,
    NoneLocation
  };
  LocationType locationType;

  AbstractLocation() : locationType(NoneLocation) {}
  AbstractLocation(LocationType locationType_) : locationType(locationType_) {}

public:
  virtual void dump() const = 0;
  virtual void fullDump(AnalysisManager* Mgr) const = 0;
  virtual ~AbstractLocation() {}
};

class AbstractStmt : public AbstractLocation {
  const Stmt* S;
  size_t no;
  AbstractBlock* parent;

public:
  AbstractStmt(const Stmt* S_, size_t no_, AbstractBlock* parent_)
      : AbstractLocation(StmtLocation), S(S_), no(no_), parent(parent_) {}

  AbstractBlock& getParent() { return *parent; }

  AbstractStmt* getStmtKey() { return this; }

  void dump() const { llvm::errs() << "S:" << no << "\n"; }

  void fullDump(AnalysisManager* Mgr) const {
    llvm::errs() << "S:" << no << " ";
    printStmt(S, *Mgr, "\ts");
  }

  const Stmt* getStmt() const {
    assert(S && "non valid stmt");
    return S;
  }

  ~AbstractStmt() {}
};

class AbstractBlock : public AbstractLocation {
private:
  enum BlockType { EntryBlock, ExitBlock, NormalBlock };
  using Stmts = std::vector<AbstractStmt>;
  using Blocks = std::vector<AbstractBlock*>;

  // block data
  const CFGBlock* block;
  AbstractFunction* function;
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
  AbstractBlock(const CFGBlock* block_, AbstractFunction* function_)
      : block(block_), function(function_) {
    blockType = getBlockType(block_);
    blockNo = getBlockId(block_);
    locationType = BlockLocation;
  }

  static size_t getBlockId(const CFGBlock* block_) {
    const CFG* cfg = block_->getParent();
    size_t numBlocks = cfg->size();
    return numBlocks - block_->getBlockID() - 1;
  }

  AbstractFunction& getParent() { return *function; }

  bool isEntryExit() const { return blockType != NormalBlock; }

  bool isLastStmt(const Stmt* S) {
    auto& lastStmt = stmts.back();
    return lastStmt.getStmt() == S;
  }

  void insertStmt(const Stmt* S) {
    if (isEntryExit()) {
      llvm::report_fatal_error("disable adding stmt to entry exit block");
    }
    size_t no = stmts.size();
    stmts.emplace_back(S, no, this);
  }

  void insertSucc(AbstractBlock* block_) { succs.push_back(block_); }

  void insertPred(AbstractBlock* block_) { preds.push_back(block_); }

  CFGElement getLastElement() { return block->back(); }

  auto getCFGElements() const {
    return llvm::iterator_range(block->begin(), block->end());
  }

  auto getStmts() { return llvm::iterator_range(stmts.begin(), stmts.end()); }

  auto getCFGSuccessors() const {
    return llvm::iterator_range(block->succ_begin(), block->succ_end());
  }

  auto getSuccessors() {
    return llvm::iterator_range(succs.begin(), succs.end());
  }

  auto getCFGPredecessors() const {
    return llvm::iterator_range(block->pred_begin(), block->pred_end());
  }

  auto getPredecessors() {
    return llvm::iterator_range(preds.begin(), preds.end());
  }

  AbstractBlock* getBlockEntryKey() { return this; }

  AbstractStmt* getBlockExitKey() { return &stmts.back(); }

  size_t numStmts() const { return stmts.size(); }

  void dump() const { llvm::errs() << "B:" << blockNo; }

  void fullDump(AnalysisManager* Mgr) const {
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

    llvm::errs() << "B:" << blockNo << "\n";
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

    for (auto stmt : stmts) {
      stmt.dump();
    }
  }

  ~AbstractBlock() {}
};

class AbstractFunction : public AbstractLocation {
  using Blocks = std::vector<AbstractBlock>;

  const FunctionDecl* function;
  const CFG* cfg;

  Blocks blocks;

public:
  void initFunction(const FunctionDecl* function_, AnalysisManager* Mgr) {
    function = function_;
    cfg = Mgr->getCFG(function);
    locationType = FunctionLocation;
  }

  AbstractBlock& insertBlock(const CFGBlock* block) {
    blocks.emplace_back(block, this);
    return blocks.back();
  }

  auto getCFGBlocks() const {
    return llvm::iterator_range(cfg->rbegin(), cfg->rend());
  }

  auto getBlocks() {
    return llvm::iterator_range(blocks.begin(), blocks.end());
  }

  AbstractBlock* getBlock(size_t no) { return &blocks[no]; }

  AbstractBlock* getEntryBlockKey() { return &blocks.front(); }

  AbstractFunction* getFunctionEntryKey() { return this; }

  AbstractStmt* getFunctionExitKey() {
    auto& lastBlock = blocks.back();
    return lastBlock.getBlockExitKey();
  }

  size_t numBlocks() const { return blocks.size(); }

  const FunctionDecl* getFunction() const { return function; }

  void fullDump(AnalysisManager* Mgr) const {
    printND(function, "F:");
    for (auto& block : blocks) {
      block.fullDump(Mgr);
    }
  }

  void dump() const { printND(function, "F:"); }
};

class AbstractProgram {
  using AbstractFunctionMap = std::map<const FunctionDecl*, AbstractFunction>;

  AbstractFunctionMap abstractFunctionMap;

public:
  AbstractFunction& insertAbstractFunction(const FunctionDecl* function,
                                           AnalysisManager* Mgr) {
    auto& abstractFunction = abstractFunctionMap[function];
    abstractFunction.initFunction(function, Mgr);
    return abstractFunction;
  }

  AbstractFunction& getAbstractFunction(const FunctionDecl* function) {
    assert(abstractFunctionMap.count(function));
    return abstractFunctionMap[function];
  }

  size_t numFunctions() const { return abstractFunctionMap.size(); }

  void dump(AnalysisManager* Mgr) const {
    llvm::errs() << "Num functions: " << numFunctions() << "\n";
    for (auto& [_, abstractFunction] : abstractFunctionMap) {
      abstractFunction.fullDump(Mgr);
    }
  }
};

template <typename Transitions> class AbstractProgramBuilder {
  // useful structures
  Transitions& transitions;
  AnalysisManager* Mgr;

  void addLastStmt(AbstractBlock& abstractBlock) {
    // must not be entry nor exit block
    CFGElement element = abstractBlock.getLastElement();
    int kind = element.getKind();

    switch (kind) {
    case CFGElement::Statement: {
      CFGStmt CS = element.castAs<CFGStmt>();
      const Stmt* S = CS.getStmt();
      assert(S);
      if (!abstractBlock.isLastStmt(S)) {
        abstractBlock.insertStmt(S);
      }

    } break;
    default: {
      llvm::errs() << "kind:" << kind << "\n";
      abstractBlock.fullDump(Mgr);
      llvm::report_fatal_error("kind not covered");
    } break;
    }
  }

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

    // get final stmt
    addLastStmt(abstractBlock);
  }

  void buildSuccPred(AbstractFunction& parent, AbstractBlock& abstractBlock) {
    for (const CFGBlock* predBlock : abstractBlock.getCFGPredecessors()) {
      size_t no = AbstractBlock::getBlockId(predBlock);
      auto* predAbstractBlock = parent.getBlock(no);
      abstractBlock.insertPred(predAbstractBlock);
    }

    for (const CFGBlock* succBlock : abstractBlock.getCFGSuccessors()) {
      size_t no = AbstractBlock::getBlockId(succBlock);
      auto* succAbstractBlock = parent.getBlock(no);
      abstractBlock.insertSucc(succAbstractBlock);
    }
  }

  void buildAbstractFunction(AbstractFunction& absFunction) {
    // per function abstract cfg

    // iterate cfg
    for (const CFGBlock* block : absFunction.getCFGBlocks()) {
      // per block abstract block
      auto& abstractBlock = absFunction.insertBlock(block);
      buildAbstractBlock(abstractBlock);
    }

    for (auto& abstractBlock : absFunction.getBlocks()) {
      // nuild succ and pred
      buildSuccPred(absFunction, abstractBlock);
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
      auto& abstractCfg = abstractProgram.insertAbstractFunction(function, Mgr);
      buildAbstractFunction(abstractCfg);
    }
  }
};

} // namespace clang::ento::nvm
