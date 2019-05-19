#pragma once
#include "Common.h"
#include "StateInfo.h"
#include "States.h"

namespace clang::ento::nvm::IpSpace {

void recurseAddBase(ProgramStateRef& State, VarInfo Alias,
                          std::vector<VarInfo>& vList) {
  vList.push_back(Alias);
  //in IPA only obj->field and obj->obj are tracked
  if (Alias.hasField()) {
    //obj track
    auto Obj = VarInfo::getVarInfo(Alias);
    recurseAddBase(State, Obj, vList);
  } else {
    // alias track
    const VarInfo* VarVI = State->get<IpVarMap>(Alias);
    if (VarVI) {
      recurseAddBase(State, *VarVI, vList);
    }
  }
}

std::vector<VarInfo> getVarBaseList(ProgramStateRef& State,
                                          VarInfo Alias) {
  std::vector<VarInfo> vList;
  recurseAddBase(State, Alias, vList);
  return vList;
}

void addToVarMap(ProgramStateRef& State, VarInfo Alias, VarInfo Var) {
  const VarInfo* TransitiveVar = State->get<IpVarMap>(Var);
  if (TransitiveVar) {
    State = State->set<IpVarMap>(Alias, *TransitiveVar);
  } else {
    State = State->set<IpVarMap>(Alias, Var);
  }
}

VarInfo getVarFromMap(ProgramStateRef& State, VarInfo Alias) {
  const VarInfo* Var = State->get<IpVarMap>(Alias);
  if (Var) {
    return *Var;
  } else {
    return Alias;
  }
}

bool removeVarsFromMap(ProgramStateRef& State, const FunctionDecl* FD) {
  bool stateChanged = false;
  for (auto& [Alias, _] : State->get<IpVarMap>()) {
    if (Alias.isSameFnc(FD)) {
      State = State->remove<IpVarMap>(Alias);
      stateChanged = true;
    }
  }
  return stateChanged;
}

void printVarMap(ProgramStateRef& State) {
  for (auto& [Alias, Var] : State->get<IpVarMap>()) {
    llvm::errs() << "Alias: ";
    Alias.dump();
    llvm::errs() << "Var: ";
    Var.dump();
  }
}

} // namespace clang::ento::nvm::IpSpace