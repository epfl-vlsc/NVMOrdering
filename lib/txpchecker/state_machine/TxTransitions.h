#pragma once
#include "Common.h"
#include "DbgState.h"
#include "StateInfo.h"
#include "States.h"

namespace clang::ento::nvm::TxSpace {

bool inTx(ProgramStateRef& State) {
  DBG("inTx")
  unsigned txCount = State->get<TxCounter>();
  return txCount > 0;
}

void begTx(StateInfo& SI) {
  DBG("begTx txCount:" << txCount)
  ProgramStateRef& State = SI.State;
  unsigned txCount = State->get<TxCounter>();
  txCount += 1;

  State = State->set<TxCounter>(txCount);
  SI.stateChanged = true;
}

void endTx(StateInfo& SI) {
  DBG("endTx txCount:" << txCount)
  ProgramStateRef& State = SI.State;

  unsigned txCount = State->get<TxCounter>();
  txCount -= 1;

  if (txCount < 0) {
    // more tx end
    SI.reportNotTxPairBug();
  } else {
    State = State->set<TxCounter>(txCount);
    SI.stateChanged = true;
  }
}

} // namespace clang::ento::nvm::TxSpace