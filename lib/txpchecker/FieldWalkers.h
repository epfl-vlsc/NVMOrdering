#pragma once
#include "Common.h"
#include "FunctionInfos.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace clang::ento::nvm {

class RangeWalker : public RecursiveASTVisitor<RangeWalker> {
  bool hasMemberExpr;
  ValueDecl* VD;
public:
  RangeWalker():hasMemberExpr(false), VD(nullptr){}

  const ValueDecl* getVD(){
    return (const ValueDecl*)VD;
  }

  bool VisitMemberExpr(const MemberExpr* ME) {
    if(!hasMemberExpr){
      VD = ME->getMemberDecl();
      hasMemberExpr = true;
    }
    
    // continue traversal
    return true;
  }

  bool hasME(){
    return hasMemberExpr;
  }
}; // namespace clang::ento::nvm

} // namespace clang::ento::nvm