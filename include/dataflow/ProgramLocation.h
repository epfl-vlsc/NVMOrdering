#pragma once
#include "Common.h"

namespace clang::ento::nvm {

class ProgramLocation {
  enum PlType { FncType, BlockType, BlockEndType, ElementType, None };
  PlType plType;
  const CFG* function;
  const CFGBlock* block;
  const Stmt* blockEnd;
  const CFGElement* element;

public:
  ProgramLocation(const CFG* function_)
      : function(function_), plType(FncType) {}
  ProgramLocation(const CFGBlock* block_) : block(block_), plType(BlockType) {}
  ProgramLocation(const Stmt* blockEnd_)
      : blockEnd(blockEnd_), plType(BlockEndType) {}
  ProgramLocation(const CFGElement* element_)
      : element(element_), plType(ElementType) {}
  ProgramLocation() : plType(None) {}

  bool operator<(const ProgramLocation& X) const {
    switch (plType) {
    case FncType:
      return (function < X.function);
    case BlockType:
      return (block < X.block);
    case BlockEndType:
      return (blockEnd < X.blockEnd);
      break;
    case ElementType:
      return (element < X.element);
      break;
    case None:
      return X.plType == None;
      break;
    }
  }

  bool operator==(const ProgramLocation& X) const {
    switch (plType) {
    case FncType:
      return (function == X.function);
    case BlockType:
      return (block == X.block);
    case BlockEndType:
      return (blockEnd == X.blockEnd);
      break;
    case ElementType:
      return (element == X.element);
      break;
    case None:
      return X.plType == None;
      break;
    }
  }
}; // namespace clang::ento::nvm

class PlContext {
  ProgramLocation caller;
  ProgramLocation callee;

public:
};

class Forward {
public:
  static auto getInstructions(const CFGBlock* block) {
    return llvm::iterator_range<decltype(block->begin())>(*block);
  }

  static auto getBasicBlocks(const CFG* function) {
    return llvm::iterator_range<decltype(function->begin())>(*function);
  }

  static ProgramLocation getEntryKey(const CFGBlock* block) {
    return ProgramLocation(block);
  }

  static ProgramLocation getExitKey(const CFGBlock* block) {
    const Stmt* blockEnd = block->getTerminatorStmt();
    return ProgramLocation(blockEnd);
  }

  static auto getSuccessors(const CFGBlock* block) {
    return llvm::iterator_range<decltype(block->succ_begin())>(*block);
  }

  static auto getPredecessors(const CFGBlock* block) {
    return llvm::iterator_range<decltype(block->pred_begin())>(*block);
  }

  /*
  template <class State, class Transfer, class Meet>
  static bool prepareSummaryState(llvm::CallSite cs, llvm::Function* callee,
                                  State& state, State& summaryState,
                                  Transfer& transfer, Meet& meet) {
    unsigned index = 0;
    bool needsUpdate = false;
    for (auto& functionArg : callee->args()) {
      auto* passedConcrete = cs.getArgument(index);
      auto passedAbstract = state.find(passedConcrete);
      if (passedAbstract == state.end()) {
        transfer(*passedConcrete, state);
        passedAbstract = state.find(passedConcrete);
      }
      auto& arg = summaryState[&functionArg];
      auto newState = meet({passedAbstract->second, arg});
      needsUpdate |= !(newState == arg);
      arg = newState;
      ++index;
    }
    return needsUpdate;
  }
  */
};

} // namespace clang::ento::nvm