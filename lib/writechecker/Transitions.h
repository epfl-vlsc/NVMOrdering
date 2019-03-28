/*Only responsible for state transitions*/
#pragma once
#include "Common.h"
#include "States.h"
#include "TypeInfos.h"

namespace clang::ento::nvm {
/*
bool dclWriteDataTrans(ProgramStateRef& State, const DeclaratorDecl* D,
                       DataInfo* DI) {
  const DclState* SS = State->get<DclMap>(D);

  // check if in correct state
  if (!SS || SS->isPFenceCheck()) {
    // update to WD state
    State = State->set<DclMap>(D, DclState::getWriteData(DI));
    return true;
    // llvm::outs() << "dcl WD\n";
  } else {
    // todo throw bug
    return false;
  }
}

bool sclWriteDataTrans(ProgramStateRef& State, const DeclaratorDecl* D,
                       DataInfo* DI) {
  const SclState* SS = State->get<SclMap>(D);

  // check if in correct state
  if (!SS || SS->isWriteCheck()) {
    // update to WD state
    State = State->set<SclMap>(D, SclState::getWriteData(DI));
    return true;
    // llvm::outs() << "scl WD\n";
  } else {
    // todo throw bug
    return false;
  }
}

bool dclWriteCheckTrans(ProgramStateRef& State, const StringRef& checkName, bool& seen) {
  // iterate for dcl
  bool stateModified = false;
  for (auto& [dataDeclDecl, dclState] : State->get<DclMap>()) {
    if (dclState.isSameCheckName(checkName)) {
      seen = true;
      // update state
      if (dclState.isPFenceData()) {
        // update to WC
        DataInfo* DI = dclState.getDataInfo();
        State = State->set<DclMap>(dataDeclDecl, DclState::getWriteCheck(DI));
        stateModified = true;
        // llvm::outs() << "dcl WC\n";
      } else {
        // todo throw bug
      }
    } else {
      // todo throw bug
    }
  }
  return stateModified;
}

bool sclWriteCheckTrans(ProgramStateRef& State, const StringRef& checkName, bool& seen) {
  // iterate for scl
  bool stateModified = false;
  for (auto& [dataDeclDecl, sclState] : State->get<SclMap>()) {
    if (sclState.isSameCheckName(checkName)) {
      seen = true;
      // update state
      if (sclState.isVFenceData()) {
        // update to WC
        DataInfo* DI = sclState.getDataInfo();
        State = State->set<SclMap>(dataDeclDecl, SclState::getWriteCheck(DI));
        stateModified = true;
        // llvm::outs() << "scl WC\n";
      } else {
        // todo throw bug
      }
    } else {
      // todo throw bug
    }
  }
  return stateModified;
}

bool sclWriteMaskTrans(ProgramStateRef& State, const DeclaratorDecl* D,
                       CheckDataInfo* CDI) {
  const SclState* SS = State->get<SclMap>(D);

  // check if in correct state
  if (!SS || SS->isWriteCheck()) {
    // todo leak of old state DI
    DataInfo* DI = CDI->getDI();

    // update to WD state
    State = State->set<SclMap>(D, SclState::getWriteData(DI));
    return true;
    // llvm::outs() << "scl mask WD\n";
  } else {
    // todo throw bug
    return false;
  }
}

bool dclFlushDataTrans(ProgramStateRef& State, const DeclaratorDecl* D,
                       DataInfo* DI) {
  // different cache line
  const DclState* SS = State->get<DclMap>(D);

  // check if in correct state
  if (SS->isWriteData()) {
    // update to WD state
    State = State->set<DclMap>(D, DclState::getFlushData(DI));
    return true;
    // llvm::outs() << "FD\n";
  } else {
    // todo throw bug
    return false;
  }
}

bool dclFlushCheckTrans(ProgramStateRef& State, const StringRef& checkName) {
  bool stateModified = false;
  // iterate for dcl
  for (auto& [dataDeclDecl, dclState] : State->get<DclMap>()) {
    if (dclState.isSameCheckName(checkName)) {
      // update state
      if (dclState.isWriteCheck()) {
        // update to WC
        DataInfo* DI = dclState.getDataInfo();
        State = State->set<DclMap>(dataDeclDecl, DclState::getFlushCheck(DI));
        stateModified = true;
        // llvm::outs() << "FC\n";
      } else {
        // todo throw bug
      }
    }
  }
  return stateModified;
}

bool dclPFenceTrans(ProgramStateRef& State) {
  bool stateModified = false;
  // iterate for dcl
  for (auto& [dataDeclDecl, dclState] : State->get<DclMap>()) {
    if (dclState.isFlushData()) {
      DataInfo* DI = dclState.getDataInfo();
      State = State->set<DclMap>(dataDeclDecl, DclState::getPFenceData(DI));
      stateModified = true;
      // llvm::outs() << "PD\n";
    } else if (dclState.isFlushCheck()) {
      DataInfo* DI = dclState.getDataInfo();
      State = State->set<DclMap>(dataDeclDecl, DclState::getPFenceCheck(DI));
      stateModified = true;
      // llvm::outs() << "PC\n";
    } else {
      // todo throw bug
    }
  }
  return stateModified;
}

bool sclPFenceTrans(ProgramStateRef& State) {
  bool stateModified = false;
  // iterate for scl
  for (auto& [dataDeclDecl, sclState] : State->get<SclMap>()) {
    if (sclState.isWriteData()) {
      DataInfo* DI = sclState.getDataInfo();
      State = State->set<SclMap>(dataDeclDecl, SclState::getVFenceData(DI));
      stateModified = true;
      // llvm::outs() << "pfence VD\n";
    } else {
      // todo throw bug
    }
  }
  return stateModified;
}

bool sclVFenceTrans(ProgramStateRef& State) {
  bool stateModified = false;
  // iterate for scl
  for (auto& [dataDeclDecl, sclState] : State->get<SclMap>()) {
    if (sclState.isWriteData()) {
      DataInfo* DI = sclState.getDataInfo();
      State = State->set<SclMap>(dataDeclDecl, SclState::getVFenceData(DI));
      stateModified = true;
      // llvm::outs() << "vfence VD\n";
    } else {
      // todo throw bug
    }
  }
  return stateModified;
}
*/
} // namespace clang::ento::nvm
