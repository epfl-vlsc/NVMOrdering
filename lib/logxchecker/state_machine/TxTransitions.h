#pragma once
#include "Common.h"
#include "LogState.h"
#include "StateInfo.h"

namespace clang::ento::nvm::TxSpace {

bool inTx(ProgramStateRef& State) {
  unsigned txCount = State->get<TxCounter>();
  DBG("inTx:" << txCount)
  return txCount > 0;
}

void checkTx(StateInfo& SI) {
  ProgramStateRef& State = SI.State;
  unsigned txCount = State->get<TxCounter>();
  if (txCount < 1) {
    // more tx end
    SI.reportAccessOutsideTxBug();
  }

  DBG("checkTx txCount:" << txCount)
}

void begTx(ProgramStateRef& State, CheckerContext& C) {
  unsigned txCount = State->get<TxCounter>();
  txCount += 1;

  State = State->set<TxCounter>(txCount);
  C.addTransition(State);

  DBG("begTx txCount:" << txCount)
}

void endTx(ProgramStateRef& State, CheckerContext& C) {
  unsigned txCount = State->get<TxCounter>();
  txCount -= 1;

  State = State->set<TxCounter>(txCount);
  C.addTransition(State);

  DBG("endTx txCount:" << txCount)
}

} // namespace clang::ento::nvm::TxSpace