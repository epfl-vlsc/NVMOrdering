#pragma once
#include "Common.h"
#include "ProgramLocation.h"

namespace clang::ento::nvm {

class AbstractCfgBlock {
  using PlVec = std::vector<ProgramLocation>;

  PlVec plStmts;
  PlVec predecessors;
  PlVec successors;

public:
  void insertStmt(const Stmt* S) { plStmts.emplace_back(ProgramLocation(S)); }

  auto getPlStmts() const {
    return llvm::iterator_range(plStmts.begin(), plStmts.begin());
  }

  auto getPredecessors() const {
    return llvm::iterator_range(predecessors.begin(), predecessors.begin());
  }

  auto getSuccessors() const {
    return llvm::iterator_range(successors.begin(), successors.begin());
  }

  ProgramLocation& getEntryPlStmt() { return plStmts[0]; }

  ProgramLocation& getExitPlStmt() { return plStmts[numStmts() - 1]; }

  size_t numStmts() const { return plStmts.size(); }

  // normal constructor
  AbstractCfgBlock() {}

  // Move constructor
  AbstractCfgBlock(AbstractCfgBlock&& X) noexcept
      : plStmts(std::move(X.plStmts)), predecessors(std::move(X.predecessors)),
        successors(std::move(X.successors)) {}

  // move assignment operator
  AbstractCfgBlock& operator=(AbstractCfgBlock&& X) noexcept {
    plStmts = std::move(X.plStmts);
    predecessors = std::move(X.predecessors);
    successors = std::move(X.successors);
    return *this;
  }

  // Copy operator
  AbstractCfgBlock(const AbstractCfgBlock& X)
      : plStmts(X.plStmts), predecessors(X.predecessors),
        successors(X.successors) {}

  // assignment operator
  AbstractCfgBlock& operator=(const AbstractCfgBlock& X) {
    plStmts = X.plStmts;
    predecessors = X.predecessors;
    successors = X.successors;
    return *this;
  }

  // destructor
  ~AbstractCfgBlock() noexcept {
    plStmts.clear();
    predecessors.clear();
    successors.clear();
  }
};

class AbstractCfg {
  using PlVec = std::vector<ProgramLocation>;

  PlVec plBlocks;

public:
  void insertBlock(const CFGBlock* block) {
    plBlocks.emplace_back(ProgramLocation(block));
  }

  auto getPlBlocks() const {
    return llvm::iterator_range(plBlocks.begin(), plBlocks.begin());
  }

  ProgramLocation& getEntryPlBlock() { return plBlocks[0]; }

  ProgramLocation& getExitPlBlock() { return plBlocks[numBlocks() - 1]; }

  size_t numBlocks() const { return plBlocks.size(); }

  // normal constructor
  AbstractCfg() {}

  // Move constructor
  AbstractCfg(AbstractCfg&& X) noexcept : plBlocks(std::move(X.plBlocks)) {}

  // move assignment operator
  AbstractCfg& operator=(AbstractCfg&& X) noexcept {
    plBlocks = std::move(X.plBlocks);
    return *this;
  }

  // Copy operator
  AbstractCfg(const AbstractCfg& X) : plBlocks(X.plBlocks) {}

  // assignment operator
  AbstractCfg& operator=(const AbstractCfg& X) {
    plBlocks = X.plBlocks;

    return *this;
  }

  // destructor
  ~AbstractCfg() noexcept { plBlocks.clear(); }
};

class AbstractProgram {
  using FunctionCfgMap = std::map<const FunctionDecl*, AbstractCfg>;

  FunctionCfgMap functionCfgMap;

public:
  void addFunction(const FunctionDecl* FD, AbstractCfg& abstractCfg) {
    functionCfgMap[FD] = std::move(abstractCfg);
  }

  size_t numFunctions() const { return functionCfgMap.size(); }

  AbstractProgram() {}

  // Move constructor
  AbstractProgram(AbstractProgram&& X) noexcept
      : functionCfgMap(std::move(X.functionCfgMap)) {}

  // move assignment operator
  AbstractProgram& operator=(AbstractProgram&& X) noexcept {
    functionCfgMap = std::move(X.functionCfgMap);
    return *this;
  }

  // Copy operator
  AbstractProgram(const AbstractProgram& X) : functionCfgMap(X.functionCfgMap) {}

  // assignment operator
  AbstractProgram& operator=(const AbstractProgram& X) {
    functionCfgMap = X.functionCfgMap;

    return *this;
  }

  // destructor
  ~AbstractProgram() noexcept { functionCfgMap.clear(); }
};

template <typename Transitions> class AbstractProgramBuilder {
  // useful structures
  Transitions& transitions;
  AnalysisManager* Mgr;

public:
  AbstractProgramBuilder(Transitions& transitions_): transitions(transitions_) {
    Mgr = transitions.getMgr();
    assert(Mgr);
  }

  AbstractProgram buildProgram() {
    AbstractProgram abstractProgram;

    /*
    for (const FunctionDecl* FD : transitions.getAnalysisFunctions()) {
      AbstractCfg abstractCfg;
    }
    */

    return abstractProgram;
  }
};

} // namespace clang::ento::nvm
