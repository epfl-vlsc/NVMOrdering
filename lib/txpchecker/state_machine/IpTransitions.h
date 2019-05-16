#pragma once
#include "Common.h"
#include "StateInfo.h"
#include "States.h"

namespace clang::ento::nvm::IpSpace {

void setIpMap(ProgramStateRef& State, const NamedDecl* Param,
              const NamedDecl* Arg) {
  const NamedDecl* const* TransitiveArg = State->get<IpMap>(Arg);
  if (TransitiveArg) {
    DBG("Param: " << Param->getNameAsString())
    DBG("TransitiveArg: " << (*TransitiveArg)->getNameAsString())
    State = State->set<IpMap>(Param, *TransitiveArg);
  } else {
    DBG("Param: " << Param->getNameAsString())
    DBG("Arg: " << Arg->getNameAsString())
    State = State->set<IpMap>(Param, Arg);
  }
}

const NamedDecl* getRealND(ProgramStateRef& State, const NamedDecl* Param) {
  const NamedDecl* const* Arg = State->get<IpMap>(Param);
  if (Arg) {
    return *Arg;
  } else {
    return Param;
  }
}

} // namespace clang::ento::nvm::IpSpace