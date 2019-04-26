#pragma once
#include "Common.h"
#include "DbgState.h"
#include "StateInfo.h"
#include "States.h"

namespace clang::ento::nvm::TxSpace {

bool inTx(ProgramStateRef& State) {
  unsigned txCount = State->get<TxCounter>();
  DBG("inTx:" << txCount)
  return txCount > 0;
}

void begTx(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  unsigned txCount = State->get<TxCounter>();
  txCount += 1;

  State = State->set<TxCounter>(txCount);
  SI.stateChanged = true;

  DBG("begTx txCount:" << txCount)
}

void endTx(StateInfo& SI) {
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

  DBG("endTx txCount:" << txCount)
}

} // namespace clang::ento::nvm::TxSpace