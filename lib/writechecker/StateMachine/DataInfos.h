#pragma once
#include "Common.h"
#include "AnnotationInfos.h"

namespace clang::ento::nvm {

class VarInfos {
  using InfoList = std::vector<BaseInfo*>;
  using ValueMap = std::map<const char*, InfoList>;
  ValueMap usedVars;

public:
  InfoList& getInfoList(const ValueDecl* VD){
    const char* D = (const char*) VD;
    assert(usedVars.count(D));
    return usedVars[D];
  }

  bool isUsedVar(const ValueDecl* VD){
    const char* D = (const char*) VD;
    return usedVars.count(D);
  }

  void addUsedVar(const AnnotateAttr* AA, BaseInfo* BI){
    const char* D = (const char*) AA;
    addUsedVar(D, BI);
  }

  void addUsedVar(const ValueDecl* VD, BaseInfo* BI){
    const char* D = (const char*) VD;
    addUsedVar(D, BI);
  }

  void addUsedVar(const char* D, BaseInfo* BI){
    if (!usedVars.count(D)) {
      // not exist
      usedVars[D];
    }
    usedVars[D].push_back(BI);
  }

  void dump() {
    for (auto& [D, BIV] : usedVars) {
      const Decl* BD = (const Decl*) D;
      if (const ValueDecl* VD = dyn_cast_or_null<ValueDecl>(BD)) {
        llvm::outs() << VD->getQualifiedNameAsString() << "\n";
      }

      for (auto& BI : BIV) {
        llvm::outs() << "\t";
        BI->dump();
        llvm::outs() << "\n";
      }
    }
  }
};

} // namespace clang::ento::nvm