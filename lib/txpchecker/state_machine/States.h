#pragma once
#include "Common.h"

namespace clang::ento::nvm {

struct VarInfo {
private:
  const FunctionDecl* Fnc;
  const NamedDecl* Obj;
  const NamedDecl* Field;

  VarInfo(const FunctionDecl* Fnc_, const NamedDecl* Obj_,
          const NamedDecl* Field_)
      : Fnc(Fnc_), Obj(Obj_), Field(Field_) {
    assert(Fnc && Obj && "Var info not init properly");
  }

public:
  static VarInfo getVarInfo(const FunctionDecl* Fnc_, const NamedDecl* Obj_,
                            const NamedDecl* Field_) {
    return VarInfo(Fnc_, Obj_, Field_);
  }
  bool operator==(const VarInfo& X) const {
    return Fnc == X.Fnc && Obj == X.Obj && Field == X.Field;
  }
  bool operator<(const VarInfo& X) const {
    if (Fnc < X.Fnc && Obj < X.Obj && Field < X.Field) {
      return true;
    }
    return false;
  }

  bool isSameFnc(const FunctionDecl* FD) const { return FD == Fnc; }

  void Profile(llvm::FoldingSetNodeID& ID) const {
    ID.AddPointer(Fnc);
    ID.AddPointer(Obj);
    ID.AddPointer(Field);
  }
  void dump() const {
    llvm::errs() << "VI - fnc:" << Fnc->getNameAsString()
                 << " obj:" << Obj->getNameAsString();
    if (Field) {
      llvm::errs() << " field:" << Field->getNameAsString();
    }
    llvm::errs() << "\n";
  }

  void dump(const char* msg) const {
    llvm::errs() << msg << " ";
    dump();
  }
};

} // namespace clang::ento::nvm

REGISTER_TRAIT_WITH_PROGRAMSTATE(TxCounter, unsigned)
REGISTER_MAP_WITH_PROGRAMSTATE(LogMap, const clang::NamedDecl*, bool)
REGISTER_MAP_WITH_PROGRAMSTATE(IpMap, const clang::NamedDecl*,
                               const clang::NamedDecl*)

REGISTER_SET_WITH_PROGRAMSTATE(LogVarMap, clang::ento::nvm::VarInfo)
REGISTER_MAP_WITH_PROGRAMSTATE(IpVarMap, clang::ento::nvm::VarInfo,
                               clang::ento::nvm::VarInfo)