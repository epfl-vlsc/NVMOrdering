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

bool recReadDataTrans(CheckerContext& C, const DeclaratorDecl* DD,
                      DataInfo* DI) {
  ProgramStateRef State = C.getState();
  bool checked = false;
  for (auto& [checkDD, recState] : State->get<RecMap>()) {
    StringRef checkName = checkDD->getName();
    if (DI->isSameCheckName(checkName)) {
      // pair
      checked = true;
      if (recState.isReadCheck()) {
        CheckInfo* CI = recState.getCheckInfo();
        State = State->set<RecMap>(DD, RecState::getReadData(CI));
        C.addTransition(State);
      } else {
        // check already read
      }
    } else {
      // not pair, not checked
    }
  }

  return checked;
}

} // namespace clang::ento::nvm
