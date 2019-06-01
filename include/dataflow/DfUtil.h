#pragma once
#include "Common.h"

namespace clang::ento::nvm {

class ProgramLocation {
  enum PlType { FncType, BlockType, ElementType };
  PlType plType;
  const CFG* cfg;
  const CFGBlock* block;
  const CFGElement* element;

public:
  ProgramLocation(const CFG* cfg_) : cfg(cfg_), plType(FncType) {}
  ProgramLocation(const CFGBlock* block_) : block(block_), plType(BlockType) {}
  ProgramLocation(const CFGElement* element_)
      : element(element_), plType(ElementType) {}

  bool operator<(const ProgramLocation& X) const {
    switch (plType) {
    case FncType:
      return (cfg < X.cfg);
    case BlockType:
      return (block < X.block);
    case ElementType:
      return (element < X.element);
      break;
    default:
      llvm::report_fatal_error("wrong type of pl");
      break;
    }
  }

  bool operator==(const ProgramLocation& X) const {
    switch (plType) {
    case FncType:
      return (cfg == X.cfg);
    case BlockType:
      return (block == X.block);
    case ElementType:
      return (element == X.element);
      break;
    default:
      llvm::report_fatal_error("wrong type of pl");
      break;
    }
  }
};

/*
class FunctionContext {
  const Stmt* Caller;
  const Stmt* Callee;

public:
  FunctionContext(const Stmt* Caller_, const Stmt* Callee_)
      : Caller(Caller_), Callee(Callee_) {}

  bool operator<(const Context& X) const {
    return (Caller < X.Caller && Callee < X.Callee);
  }

  bool operator==(const Context& X) const {
    return (Caller == X.Caller && Callee == X.Callee);
  }
};
*/

} // namespace clang::ento::nvm