#pragma once
#include "Common.h"

namespace clang::ento::nvm {

class AbstractStmt;
class AbstractBlock;
class AbstractFunction;
class AbstractProgram;

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
  const AbstractBlock* parent;

public:
  AbstractStmt(const Stmt* S_, size_t no_, const AbstractBlock* parent_)
      : S(S_), no(no_), parent(parent_) {
    locationType = StmtLocation;
  }

  const AbstractBlock* getParent() const { return parent; }

  const AbstractLocation* getStmtKey() const { return this; }

  void dump() const {
    assert(S);
    llvm::errs() << "S:" << no << " " << this << " ";
  }

  void fullDump(AnalysisManager* Mgr) const {
    assert(S);
    llvm::errs() << "S:" << no << " " << this << " stmt key " << getStmtKey()
                 << " ";
    printStmt(S, *Mgr, "s", false);
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
  using Stmts = std::vector<const AbstractStmt*>;
  using Blocks = std::vector<const AbstractBlock*>;

  // block data
  const CFGBlock* block;
  const AbstractFunction* parent;
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
  AbstractBlock(const CFGBlock* block_, const AbstractFunction* parent_)
      : block(block_), parent(parent_) {
    blockType = getBlockType(block_);
    blockNo = getBlockId(block_);
    locationType = BlockLocation;
  }

  static size_t getBlockId(const CFGBlock* block_) {
    const CFG* cfg = block_->getParent();
    size_t numBlocks = cfg->size();
    return numBlocks - block_->getBlockID() - 1;
  }

  const AbstractFunction* getParent() const { return parent; }

  bool isEntryExit() const { return blockType != NormalBlock; }

  bool isLastStmt(const Stmt* S) const {
    const AbstractStmt* lastStmt = stmts.back();
    assert(lastStmt);
    return lastStmt->getStmt() == S;
  }

  void insertStmt(const AbstractStmt* absStmt) {
    if (isEntryExit()) {
      llvm::report_fatal_error("disable adding stmt to entry exit block");
    }
    stmts.push_back(absStmt);
  }

  void insertSucc(const AbstractBlock* block_) { succs.push_back(block_); }

  void insertPred(const AbstractBlock* block_) { preds.push_back(block_); }

  CFGElement getLastElement() const { return block->back(); }

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

  const AbstractLocation* getBlockEntryKey() const { return this; }

  const AbstractLocation* getBlockExitKey() const {
    if (isEntryExit()) {
      // return the entry
      return this;
    }

    const AbstractLocation* blockExitKey = stmts.back();
    assert(blockExitKey);
    return blockExitKey;
  }

  size_t numStmts() const { return stmts.size(); }

  void dump() const {
    assert(block && parent);
    llvm::errs() << "B:" << blockNo << " " << this << " ";
  }

  void fullDump(AnalysisManager* Mgr) const {
    assert(block && parent);
    llvm::errs() << "B:" << blockNo << " " << this << " ";
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

    llvm::errs() << "successors ";
    for (const AbstractBlock* absSucc : succs) {
      absSucc->dump();
      llvm::errs() << " ";
    }

    llvm::errs() << "predecessors ";
    for (const AbstractBlock* absPred : preds) {
      absPred->dump();
      llvm::errs() << " ";
    }

    llvm::errs() << "entry key " << getBlockEntryKey() << " ";
    llvm::errs() << "exit key " << getBlockExitKey() << " ";

    llvm::errs() << "\n";

    for (const AbstractStmt* absStmt : stmts) {
      absStmt->fullDump(Mgr);
      llvm::errs() << "\n";
    }
  }

  ~AbstractBlock() {}
};

class AbstractFunction : public AbstractLocation {
  using Blocks = std::vector<const AbstractBlock*>;

  const FunctionDecl* function;
  const CFG* cfg;
  const AbstractProgram* parent;

  Blocks blocks;

public:
  AbstractFunction(const FunctionDecl* function_,
                   const AbstractProgram* parent_, AnalysisManager* Mgr)
      : function(function_), parent(parent_) {
    cfg = Mgr->getCFG(function);
    locationType = FunctionLocation;
  }

  void insertBlock(const AbstractBlock* absBlock) {
    blocks.push_back(absBlock);
  }

  const AbstractProgram* getParent() const { return parent; }

  auto getCFGBlocks() const {
    return llvm::iterator_range(cfg->rbegin(), cfg->rend());
  }

  auto getBlocks() const {
    return llvm::iterator_range(blocks.begin(), blocks.end());
  }

  auto getReverseBlocks() const {
    return llvm::iterator_range(blocks.rbegin(), blocks.rend());
  }

  const AbstractBlock* getBlock(size_t no) const { return blocks[no]; }

  const AbstractLocation* getEntryBlockKey() const {
    const AbstractLocation* entryBlockKey = blocks.front();
    assert(entryBlockKey);
    return entryBlockKey;
  }

  const AbstractLocation* getFunctionEntryKey() const { return this; }

  const AbstractLocation* getFunctionExitKey() const {
    const AbstractLocation* functionExitBlock = blocks.back();
    assert(functionExitBlock);
    return functionExitBlock;
  }

  size_t numBlocks() const { return blocks.size(); }

  const FunctionDecl* getFunction() const { return function; }

  void fullDump(AnalysisManager* Mgr) const {
    assert(function);
    printND(function, "F", true, false);
    llvm::errs() << " " << this << "\n";
    for (const AbstractBlock* absBlock : blocks) {
      absBlock->fullDump(Mgr);
    }
  }

  void dump() const {
    assert(function);
    printND(function, "F", true, false);
    llvm::errs() << " " << this << "\n";
  }
};

class AbstractProgram {
  using Locations = std::set<const AbstractLocation*>;
  using AbstractFunctionMap =
      std::map<const FunctionDecl*, const AbstractFunction*>;

  Locations locations;
  AbstractFunctionMap abstractFunctionMap;

public:
  ~AbstractProgram() {
    for (auto& location : locations) {
      //delete location;
    }
  }

  void insertLocation(const AbstractLocation* absLocation) {
    locations.insert(absLocation);
  }

  void insertAbstractFunction(const AbstractFunction* absFunction) {
    const FunctionDecl* FD = absFunction->getFunction();
    abstractFunctionMap.insert(std::pair(FD, absFunction));
  }

  const AbstractFunction*
  getAbstractFunction(const FunctionDecl* function) const {
    assert(abstractFunctionMap.count(function));
    return abstractFunctionMap.at(function);
  }

  size_t numFunctions() const { return abstractFunctionMap.size(); }

  void dump(AnalysisManager* Mgr) const {
    llvm::errs() << "Num functions: " << numFunctions() << "\n";
    for (auto& [_, abstractFunction] : abstractFunctionMap) {
      abstractFunction->fullDump(Mgr);
    }
  }

  void dumpLocations(AnalysisManager* Mgr) const {
    for (auto& location : locations) {
      location->fullDump(Mgr);
    }
  }
};

template <typename Transitions> class AbstractProgramBuilder {
  // useful structures
  AbstractProgram& abstractProgram;
  Transitions& transitions;
  AnalysisManager* Mgr;

  void addLocation(const AbstractLocation* absLocation) {
    abstractProgram.insertLocation(absLocation);
  }

  void addLastStmt(const AbstractBlock* absBlock) {
    // must not be entry nor exit block
    CFGElement element = absBlock->getLastElement();
    int kind = element.getKind();

    switch (kind) {
    case CFGElement::Statement: {
      CFGStmt CS = element.castAs<CFGStmt>();
      const Stmt* S = CS.getStmt();
      assert(S);
      if (!absBlock->isLastStmt(S)) {
        size_t stmtNo = absBlock->numStmts();
        const AbstractStmt* absStmt = new AbstractStmt(S, stmtNo, absBlock);
        addLocation(absStmt);

        ((AbstractBlock*)absBlock)->insertStmt(absStmt);
      }

    } break;
    default: {
      llvm::errs() << "kind:" << kind << "\n";
      absBlock->fullDump(Mgr);
      llvm::report_fatal_error("kind not covered");
    } break;
    }
  }

  void buildAbstractBlock(const AbstractBlock* absBlock) {
    // skip if entry or exit block
    if (absBlock->isEntryExit()) {
      return;
    }

    // iterate blocks
    for (const CFGElement element : absBlock->getCFGElements()) {
      if (Optional<CFGStmt> CS = element.getAs<CFGStmt>()) {
        const Stmt* S = CS->getStmt();
        assert(S);

        // check if stmt is used in data flow
        auto PTI = transitions.parseStmt(S);
        if (PTI.isStmtUsed()) {
          size_t stmtNo = absBlock->numStmts();
          const AbstractStmt* absStmt = new AbstractStmt(S, stmtNo, absBlock);
          addLocation(absStmt);

          ((AbstractBlock*)absBlock)->insertStmt(absStmt);
        }
      }
    }

    // get final stmt
    addLastStmt(absBlock);
  }

  void buildSuccPred(const AbstractBlock* absBlock,
                     const AbstractFunction* parent) {
    for (const CFGBlock* predBlock : absBlock->getCFGPredecessors()) {
      size_t no = AbstractBlock::getBlockId(predBlock);
      const AbstractBlock* predAbsBlock = parent->getBlock(no);
      ((AbstractBlock*)absBlock)->insertPred(predAbsBlock);
    }

    for (const CFGBlock* succBlock : absBlock->getCFGSuccessors()) {
      size_t no = AbstractBlock::getBlockId(succBlock);
      const AbstractBlock* succAbsBlock = parent->getBlock(no);
      ((AbstractBlock*)absBlock)->insertSucc(succAbsBlock);
    }
  }

  void buildAbstractFunction(const AbstractFunction* absFunction) {
    // per function abstract cfg

    // iterate cfg
    for (const CFGBlock* block : absFunction->getCFGBlocks()) {
      // per block abstract block
      const AbstractBlock* absBlock = new AbstractBlock(block, absFunction);
      addLocation(absBlock);

      ((AbstractFunction*)absFunction)->insertBlock(absBlock);

      buildAbstractBlock(absBlock);
    }

    for (const AbstractBlock* absBlock : absFunction->getBlocks()) {
      // build succ and pred
      buildSuccPred(absBlock, absFunction);
    }
  }

public:
  AbstractProgramBuilder(AbstractProgram& abstractProgram_,
                         Transitions& transitions_)
      : abstractProgram(abstractProgram_), transitions(transitions_) {
    Mgr = transitions.getMgr();
    // set so that all effected statements are tracked, not just unit
    transitions.useGlobalTransitions();
    assert(Mgr);

    buildProgram();
  }

  void buildProgram() {
    // create abstract control flow graphs for each function
    for (const FunctionDecl* function : transitions.getAnalysisFunctions()) {
      const AbstractFunction* absFunction =
          new AbstractFunction(function, &abstractProgram, Mgr);

      addLocation(absFunction);
      abstractProgram.insertAbstractFunction(absFunction);

      buildAbstractFunction(absFunction);
    }
  }
};

class AbstractContext {
  const CallExpr* caller;
  const CallExpr* callee;

public:
  AbstractContext() : caller(nullptr), callee(nullptr) {}

  AbstractContext(const AbstractContext& X, const CallExpr* Callee) {
    caller = X.callee;
    callee = Callee;
  }

  bool operator<(const AbstractContext& X) const {
    return (caller < X.caller && callee < X.callee);
  }

  bool operator==(const AbstractContext& X) const {
    return (caller == X.caller && callee == X.callee);
  }

  void fullDump(AnalysisManager* Mgr) const {
    auto& ASTC = Mgr->getASTContext();
    llvm::errs() << "caller:";
    if (caller) {
      caller->dumpPretty(ASTC);
    } else {
      llvm::errs() << "none";
    }

    llvm::errs() << " callee:";
    if (callee) {
      callee->dumpPretty(ASTC);
    } else {
      llvm::errs() << "none";
    }
    llvm::errs() << "\n";
  }
};

} // namespace clang::ento::nvm
