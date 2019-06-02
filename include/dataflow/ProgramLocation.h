#pragma once
#include "Common.h"

namespace clang::ento::nvm {

union ProgramLocation {
  const CFG* function;
  const CFGBlock* block;
  const Stmt* blockEnd;
  const CFGElement* element;

  ProgramLocation(const CFG* pl) { function = pl; }
  ProgramLocation(const CFGBlock* pl) { block = pl; }
  ProgramLocation(const Stmt* pl) { blockEnd = pl; }
  ProgramLocation(const CFGElement* pl) { element = pl; }
  ProgramLocation() { function = nullptr; }

  bool operator<(const ProgramLocation& X) const {
    return function < X.function;
  }

  bool operator==(const ProgramLocation& X) const {
    return function == X.function;
  }
};

/*
class ProgramLocation {
  enum PlType { FncType, BlockType, BlockEndType, ElementType, None };

  Pl programLocation;
  PlType plType;

public:
  ProgramLocation(const CFG* function)
      : programLocation(function), plType(FncType) {}
  ProgramLocation(const CFGBlock* block)
      : programLocation(block), plType(BlockType) {}
  ProgramLocation(const Stmt* blockEnd)
      : programLocation(blockEnd), plType(BlockEndType) {}
  ProgramLocation(const CFGElement* element_)
      : programLocation(element_), plType(ElementType) {}
  ProgramLocation() : plType(None) {}

  bool operator<(const ProgramLocation& X) const {
    return programLocation < X.programLocation;
  }

  bool operator==(const ProgramLocation& X) const {
    return programLocation == X.programLocation;
  }
};
*/

class PlContext {
  ProgramLocation caller;
  ProgramLocation callee;

public:
};

class Forward {
public:
  static auto getInstructions(const CFGBlock* block) {
    return llvm::iterator_range(block->begin(), block->end());
  }

  static auto getBasicBlocks(const CFG* function) {
    return llvm::iterator_range(function->rbegin(), function->rend());
  }

  static auto getSuccessors(const CFGBlock* block) {
    return llvm::iterator_range(block->succ_begin(), block->succ_end());
  }

  static auto getPredecessors(const CFGBlock* block) {
    return llvm::iterator_range(block->pred_begin(), block->pred_end());
  }

  static ProgramLocation getEntryKey(const CFGBlock* block) {
    return ProgramLocation(block);
  }

  static ProgramLocation getExitKey(const CFGBlock* block) {
    CFGTerminator terminator = block->getTerminator();
    const Stmt* blockEnd = terminator.getStmt();
    return ProgramLocation(blockEnd);
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