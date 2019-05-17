#pragma once
#include "Common.h"

namespace clang::ento::nvm {

struct VarInfo {
private:
  const FunctionDecl* Fnc;
  const NamedDecl* Obj;
  const NamedDecl* Field;
  const LocationContext* LC;

  VarInfo(const FunctionDecl* Fnc_, const NamedDecl* Obj_,
          const NamedDecl* Field_, const LocationContext* LC_)
      : Fnc(Fnc_), Obj(Obj_), Field(Field_), LC(LC_) {}

  VarInfo(const FunctionDecl* Fnc_, const NamedDecl* Obj_,
          const LocationContext* LC_)
      : Fnc(Fnc_), Obj(Obj_), Field(nullptr), LC(LC_) {}

public:
  static VarInfo getVarInfo(const FunctionDecl* Fnc_, const NamedDecl* Obj_,
                            const NamedDecl* Field_,
                            const LocationContext* LC_) {
    return VarInfo(Fnc_, Obj_, Field_, LC_);
  }

  bool operator==(const VarInfo& X) const {
    return Fnc == X.Fnc && LC == X.LC && Obj == X.Obj && Field == X.Field;
  }
  void Profile(llvm::FoldingSetNodeID& ID) const {
    ID.AddPointer(Fnc);
    ID.AddPointer(Obj);
    ID.AddPointer(Field);
    ID.AddPointer(LC);
  }
};

} // namespace clang::ento::nvm

REGISTER_TRAIT_WITH_PROGRAMSTATE(TxCounter, unsigned)
REGISTER_MAP_WITH_PROGRAMSTATE(LogMap, const clang::NamedDecl*, bool)
REGISTER_MAP_WITH_PROGRAMSTATE(IpMap, const clang::NamedDecl*,
                               const clang::NamedDecl*)