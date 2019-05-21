#pragma once
#include "Common.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace clang::ento::nvm {

class AllocWalker : public RecursiveASTVisitor<AllocWalker> {
  bool isAlloc;

public:
  AllocWalker() : isAlloc(false) {}

  bool VisitCXXNewExpr(const CXXNewExpr* CNE) {
    isAlloc = true;

    // continue traversal
    return true;
  }

  bool VisitCallExpr(const CallExpr* CE) {
    if (const FunctionDecl* FD = CE->getDirectCallee()) {
      const IdentifierInfo* II = FD->getIdentifier();

      if (II->isStr("malloc") || II->isStr("calloc") || II->isStr("realloc")) {
        isAlloc = true;
      }
    }

    // continue traversal
    return true;
  }

  bool shouldVisitTemplateInstantiations() const { return true; }

  bool isAllocUsed() { return isAlloc; }

  // bool shouldVisitImplicitCode() const { return true; }
};

} // namespace clang::ento::nvm