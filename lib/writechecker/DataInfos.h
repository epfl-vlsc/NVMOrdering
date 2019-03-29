#pragma once
#include "Common.h"
#include "AnnotationInfos.h"

namespace clang::ento::nvm {

class VarInfos {
  using InfoList = std::vector<BaseInfo*>;
  using ValueMap = std::map<const ValueDecl*, InfoList>;
  ValueMap usedVars;

public:
  InfoList& getInfoList(const ValueDecl* VD){
    assert(usedVars.count(VD));
    return usedVars[VD];
  }

  bool isUsedVar(const ValueDecl* VD){
    return usedVars.count(VD);
  }

  void addUsedVar(const ValueDecl* VD, BaseInfo* BI){
    if (!usedVars.count(VD)) {
      // not exist
      usedVars[VD];
    }
    usedVars[VD].push_back(BI);
  }

  void dump() {
    for (auto& [VD, BIV] : usedVars) {
      llvm::outs() << VD->getQualifiedNameAsString() << "\n";
      for (auto& BI : BIV) {
        llvm::outs() << "\t";
        BI->dump();
        llvm::outs() << "\n";
      }
    }
  }
};

} // namespace clang::ento::nvm