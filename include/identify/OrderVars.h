#pragma once
#include "Common.h"

namespace clang::ento::nvm {

template<typename BaseInfo>
class OrderVars {
  using InfoList = std::set<BaseInfo*>;
  using ValueMap = std::map<const char*, InfoList>;
  ValueMap usedVars;

public:
  InfoList& getInfoList(const ValueDecl* VD) {
    const char* D = (const char*)VD;
    return getInfoList(D);
  }

  InfoList& getInfoList(const char* D) {
    assert(usedVars.count(D));
    return usedVars[D];
  }

  bool isUsedVar(const ValueDecl* VD) {
    const char* D = (const char*)VD;
    return usedVars.count(D);
  }

  void addUsedVar(const AnnotateAttr* AA, BaseInfo* BI) {
    const char* D = (const char*)AA;
    addUsedVar(D, BI);
  }

  void addUsedVar(const ValueDecl* VD, BaseInfo* BI) {
    const char* D = (const char*)VD;
    addUsedVar(D, BI);
  }

  void addUsedVar(const char* D, BaseInfo* BI) {
    if (!usedVars.count(D)) {
      // not exist
      usedVars[D];
    }
    usedVars[D].insert(BI);
  }

  void dump() {
    for (auto& [D, BIV] : usedVars) {
      llvm::outs() << (void*)D << "\n";

      for (auto& BI : BIV) {
        llvm::outs() << "\t";
        BI->dump();
        llvm::outs() << "\n";
      }
    }
  }
};

} // namespace clang::ento::nvm