#pragma once
#include "Common.h"
#include "FunctionInfos.h"
#include "StateMachine/DataInfos.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace clang::ento::nvm {

class MaskWalker : public ConstStmtVisitor<MaskWalker> {
  static constexpr const char* MASK = "MASK";
  bool maskUse;
  bool usesUnary;
  bool readMode;

public:
  MaskWalker(bool readMode_)
      : maskUse(false), usesUnary(false), readMode(readMode_) {}

  void VisitDeclRefExpr(const DeclRefExpr* DRE) {
    StringRef currentMask =
        DRE->getNameInfo().getName().getAsIdentifierInfo()->getName();
    // llvm::outs() << currentMask << " " << mask << "\n";
    if (currentMask.equals(MASK)) {
      maskUse = true;
    }

    VisitChildren(DRE);
  }

  void VisitStmt(const Stmt* S) { VisitChildren(S); }

  void VisitUnaryOperator(const UnaryOperator* UOp) {
    // using another mask
    usesUnary = true;

    VisitChildren(UOp);
  }

  void VisitChildren(const Stmt* S) {
    for (Stmt::const_child_iterator I = S->child_begin(), E = S->child_end();
         I != E; ++I) {
      if (const Stmt* Child = *I) {
        Visit(Child);
      }
    }
  }

  bool usesMask() {
    if (!maskUse) {
      llvm::report_fatal_error("check mask usage");
    }

    return (usesUnary ^ readMode);
  }
};

bool usesMask(const Stmt* S, bool readMode) {
  // S->dump();
  MaskWalker maskWalker(readMode);
  maskWalker.Visit(S);
  return maskWalker.usesMask();
}

} // namespace clang::ento::nvm