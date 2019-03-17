/*Only responsible for state transitions*/
#pragma once
#include "Common.h"
#include "States.h"
#include "TypeInfos.h"

namespace clang::ento::nvm {

bool recReadCheckTrans(ProgramStateRef& State, const DeclaratorDecl* DD,
                       CheckInfo* CI) {
  const RecState* RS = State->get<RecMap>(DD);

  if (!RS) {
    // if not checked, update to RC state
    State = State->set<RecMap>(DD, RecState::getReadCheck(CI));
    return true;
    // llvm::outs() << "RC\n";
  } else {
    // todo already read check
    return false;
  }
}

void recReadDataTrans(ProgramStateRef& State, const DeclaratorDecl* DD,
                       CheckInfo* CI) {
      State = State->set<RecMap>(DD, RecState::getReadData(CI));
      // llvm::outs() << "pfence VD\n";
}

} // namespace clang::ento::nvm
