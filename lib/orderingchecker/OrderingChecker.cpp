//===-- OrderingChecker.cpp -----------------------------------------*
//ensure main handle functions only add one state

#include "OrderingChecker.h"

namespace clang::ento::nvm
{

void OrderingChecker::checkBeginFunction(CheckerContext &C) const
{
  bool isAnnotated = nvmFncInfo.isAnnotatedFunction(C);
  bool isTopFnc = isTopFunction(C);

  //if not an annotated function, do not analyze
  if (!isAnnotated && isTopFnc)
  {
    ExplodedNode *ErrNode = C.generateErrorNode();
    if (!ErrNode)
      return;
  }
}

void OrderingChecker::checkEndFunction(CheckerContext &C) const
{
  const LocationContext *LC = C.getLocationContext();
  const FunctionDecl *D = getFuncDecl(LC);
  llvm::outs() << D->getName() << " end\n";

  bool isAnnotated = nvmFncInfo.isAnnotatedFunction(C);
  bool isTopFnc = isTopFunction(C);
  if (isAnnotated && isTopFnc)
  {
    //ensured it is the top function and annotated

    ProgramStateRef State = C.getState();
    //iterate over dcl
    for (auto &[dataDeclDecl, dclState] : State->get<DclMap>())
    {

      llvm::outs() << dataDeclDecl << " dcl "
                   << dataDeclDecl->getName() << " " << dclState.getStateName() << "\n";
      if (!dclState.isPFenceCheck())
      {
        ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
        if (!ErrNode)
        {
          return;
        }
        BReporter.reportModelBug(C, ErrNode, C.getBugReporter());
      }
    }

    //iterate over scl
    for (auto &[dataDeclDecl, sclState] : State->get<SclMap>())
    {
      llvm::outs() << dataDeclDecl << " scl "
                   << dataDeclDecl->getName() << " " << sclState.getStateName() << "\n";
      if (!sclState.isWriteCheck())
      {
        ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
        if (!ErrNode)
        {
          return;
        }
        BReporter.reportModelBug(C, ErrNode, C.getBugReporter());
      }
    }
  }
}

void OrderingChecker::checkDeadSymbols(SymbolReaper &SymReaper, CheckerContext &C) const
{
  //todo implement
  //llvm::outs() << "consider implementing checkDeadSymbols\n";
}

ProgramStateRef OrderingChecker::checkPointerEscape(ProgramStateRef State,
                                                    const InvalidatedSymbols &Escaped,
                                                    const CallEvent *Call,
                                                    PointerEscapeKind Kind) const
{
  //todo implement
  //llvm::outs() << "consider implementing checkPointerEscape\n";
  return State;
}

void OrderingChecker::checkBind(SVal Loc, SVal Val, const Stmt *S,
                                CheckerContext &C) const
{
  //S->dump();
  const MemRegion *Region = Loc.getAsRegion();
  if (const FieldRegion *FieldReg = Region->getAs<FieldRegion>())
  {
    const Decl *BD = FieldReg->getDecl();
    const DeclaratorDecl *D = getDeclaratorDecl(BD);

    if (nvmTypeInfo.inLabels(D))
    {
      LabeledInfo *LI = nvmTypeInfo.getDeclaratorInfo(D);
      if (LI->isCheck())
      {
        auto *I = static_cast<CheckInfo *>(LI);
        if (I->hasMask())
        {
          //bit field special case
          //can be data or check, if it is data, treat is as data
          //if it is check, treat it as check
          auto *CDI = static_cast<CheckDataInfo *>(I);
          handleWriteMask(S, C, D, CDI);
        }
        else
        {
          handleWriteCheck(C, D, I);
        }
      }
      else
      {
        auto *I = static_cast<DataInfo *>(LI);
        handleWriteData(C, D, I);
      }
    }
  }
}

void OrderingChecker::handleWriteData(CheckerContext &C, const DeclaratorDecl *D,
                                      DataInfo *DI) const
{
  ProgramStateRef State = C.getState();

  //check if not completed or not initial throw error
  if (DI->isDcl())
  {
    //different cache line
    const DclState *SS = State->get<DclMap>(D);

    //check if in correct state
    if (!SS || SS->isPFenceCheck())
    {
      //update to WD state
      State = State->set<DclMap>(D, DclState::getWriteData(DI));
      C.addTransition(State);
      //llvm::outs() << "dcl WD\n";
    }
    else
    {
      /*
      ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
      if (!ErrNode)
      {
        return;
      }
      BReporter.reportWriteDataBug(C, D, ErrNode, C.getBugReporter());
      */
    }
  }
  else
  {
    //same cache line
    const SclState *SS = State->get<SclMap>(D);

    //check if in correct state
    if (!SS || SS->isWriteCheck())
    {
      //update to WD state
      State = State->set<SclMap>(D, SclState::getWriteData(DI));
      C.addTransition(State);
      llvm::outs() << "scl WD\n";
    }
    else
    {
      /*
      ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
      if (!ErrNode)
      {
        return;
      }
      BReporter.reportWriteDataBug(C, D, ErrNode, C.getBugReporter());
      */
    }
  }
}

void OrderingChecker::handleWriteCheck(CheckerContext &C,
                                       const DeclaratorDecl *D, CheckInfo *CI) const
{
  ProgramStateRef State = C.getState();
  StringRef checkName = D->getName();
  bool seenCheck = false;

  //iterate for dcl
  for (auto &[dataDeclDecl, dclState] : State->get<DclMap>())
  {
    if (dclState.isSameCheckName(checkName))
    {
      seenCheck = true;
      //update state
      if (dclState.isPFenceData())
      {
        //update to WC
        DataInfo *DI = dclState.getDataInfo();
        State = State->set<DclMap>(dataDeclDecl, DclState::getWriteCheck(DI));
        //llvm::outs() << "dcl WC\n";
      }
      else
      {
        /*
        ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
        if (!ErrNode)
        {
          return;
        }
        BReporter.reportWriteCheckBug(C, D, ErrNode, C.getBugReporter());
        */
      }
    }
  }

  //iterate for scl
  for (auto &[dataDeclDecl, sclState] : State->get<SclMap>())
  {
    if (sclState.isSameCheckName(checkName))
    {
      seenCheck = true;
      //update state
      if (sclState.isVFenceData())
      {
        //update to WC
        DataInfo *DI = sclState.getDataInfo();
        State = State->set<SclMap>(dataDeclDecl, SclState::getWriteCheck(DI));
        llvm::outs() << "scl WC\n";
      }
      else
      {
        /*
        ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
        if (!ErrNode)
        {
          return;
        }
        BReporter.reportWriteCheckBug(C, D, ErrNode, C.getBugReporter());
        */
      }
    }
  }

  if (!seenCheck)
  {
    ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
    if (!ErrNode)
    {
      return;
    }
    BReporter.reportWriteCheckBug(C, D, ErrNode, C.getBugReporter());
  }
  else
  {
    //add modified state
    C.addTransition(State);
  }
}

void OrderingChecker::handleWriteMask(
    const Stmt *S, CheckerContext &C,
    const DeclaratorDecl *D, CheckDataInfo *CDI) const
{
  if (usesMask(S, CDI->getMask()))
  {
    //access validity part
    auto *I = static_cast<CheckInfo *>(CDI);
    handleWriteCheck(C, D, I);
  }
  else
  {

    //access data part - guaranteeed to be scl
    ProgramStateRef State = C.getState();
    const SclState *SS = State->get<SclMap>(D);

    //check if in correct state
    if (!SS || SS->isWriteCheck())
    {
      //todo leak of old state DI
      DataInfo *DI = CDI->getDI();

      //update to WD state
      State = State->set<SclMap>(D, SclState::getWriteData(DI));
      C.addTransition(State);
      llvm::outs() << "scl mask WD\n";
    }
    else
    {
      /*
      ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
      if (!ErrNode)
      {
        return;
      }
      BReporter.reportWriteDataBug(C, D, ErrNode, C.getBugReporter());
      */
    }
  }
}

void OrderingChecker::checkPreCall(const CallEvent &Call,
                                   CheckerContext &C) const
{
  if (nvmFncInfo.isFlushFunction(Call))
  {
    handleFlush(Call, C);
  }
  else if (nvmFncInfo.isPFenceFunction(Call))
  {
    handlePFence(Call, C);
  }
  else if (nvmFncInfo.isVFenceFunction(Call))
  {
    handleVFence(Call, C);
  }
}

void OrderingChecker::handleFlush(const CallEvent &Call,
                                  CheckerContext &C) const
{
  if (Call.getNumArgs() > 2)
  {
    llvm::report_fatal_error("check flush function");
    return;
  }

  SVal Loc = Call.getArgSVal(0);
  const MemRegion *Region = Loc.getAsRegion();
  if (const FieldRegion *FieldReg = Region->getAs<FieldRegion>())
  {
    const Decl *BD = FieldReg->getDecl();
    const DeclaratorDecl *D = getDeclaratorDecl(BD);
    if (nvmTypeInfo.inLabels(D))
    {
      LabeledInfo *LI = nvmTypeInfo.getDeclaratorInfo(D);
      if (LI->isCheck())
      {
        auto *I = static_cast<CheckInfo *>(LI);
        handleFlushCheck(C, D, I);
      }
      else
      {
        auto *I = static_cast<DataInfo *>(LI);
        handleFlushData(C, D, I);
      }
    }
  }
}

void OrderingChecker::handleFlushData(CheckerContext &C, const DeclaratorDecl *D,
                                      DataInfo *DI) const
{
  ProgramStateRef State = C.getState();

  //check if not completed or not initial throw error
  if (DI->isDcl())
  {
    //different cache line
    const DclState *SS = State->get<DclMap>(D);

    //check if in correct state
    if (SS->isWriteData())
    {
      //update to WD state
      State = State->set<DclMap>(D, DclState::getFlushData(DI));
      C.addTransition(State);
      //llvm::outs() << "FD\n";
    }
    else
    {
      /*
      ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
      if (!ErrNode)
      {
        return;
      }
      BReporter.reportFlushDataBug(C, D, ErrNode, C.getBugReporter());
      */
    }
  }
}

void OrderingChecker::handleFlushCheck(CheckerContext &C, const DeclaratorDecl *D,
                                       CheckInfo *CI) const
{
  ProgramStateRef State = C.getState();
  StringRef checkName = D->getName();
  bool seenCheck = false;

  //iterate for dcl
  for (auto &[dataDeclDecl, dclState] : State->get<DclMap>())
  {
    if (dclState.isSameCheckName(checkName))
    {
      seenCheck = true;
      //update state
      if (dclState.isWriteCheck())
      {

        //update to WC
        DataInfo *DI = dclState.getDataInfo();
        State = State->set<DclMap>(dataDeclDecl, DclState::getFlushCheck(DI));
        //llvm::outs() << "FC\n";
      }
      else
      {
        /*
        ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
        if (!ErrNode)
        {
          return;
        }
        BReporter.reportFlushCheckBug(C, D, ErrNode, C.getBugReporter());
        */
      }
    }
  }

  if (!seenCheck)
  {
    /*
    ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
    if (!ErrNode)
    {
      return;
    }
    BReporter.reportFlushCheckBug(C, D, ErrNode, C.getBugReporter());
    */
  }
  else
  {
    C.addTransition(State);
  }
}

void OrderingChecker::handlePFence(const CallEvent &Call,
                                   CheckerContext &C) const
{
  ProgramStateRef State = C.getState();
  bool stateModified = false;
  //iterate for dcl
  for (auto &[dataDeclDecl, dclState] : State->get<DclMap>())
  {
    if (dclState.isFlushData())
    {
      DataInfo *DI = dclState.getDataInfo();
      State = State->set<DclMap>(dataDeclDecl, DclState::getPFenceData(DI));
      stateModified = true;
      //llvm::outs() << "PD\n";
    }
    else if (dclState.isFlushCheck())
    {
      DataInfo *DI = dclState.getDataInfo();
      State = State->set<DclMap>(dataDeclDecl, DclState::getPFenceCheck(DI));
      stateModified = true;
      //llvm::outs() << "PC\n";
    }
    else
    {
      /*
      ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
      if (!ErrNode)
      {
        return;
      }
      BReporter.reportFenceBug(Call, C, ErrNode, C.getBugReporter());
      */
    }
  }

  //iterate for scl
  for (auto &[dataDeclDecl, sclState] : State->get<SclMap>())
  {
    if (sclState.isWriteData())
    {
      DataInfo *DI = sclState.getDataInfo();
      State = State->set<SclMap>(dataDeclDecl, SclState::getVFenceData(DI));
      stateModified = true;
      llvm::outs() << "pfence VD\n";
    }
    else
    {
      /*
      ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
      if (!ErrNode)
      {
        return;
      }
      BReporter.reportFenceBug(Call, C, ErrNode, C.getBugReporter());
      */
    }
  }

  if (stateModified)
  {
    C.addTransition(State);
  }
}

void OrderingChecker::handleVFence(const CallEvent &Call,
                                   CheckerContext &C) const
{
  ProgramStateRef State = C.getState();
  bool stateModified = false;
  //iterate for scl
  for (auto &[dataDeclDecl, sclState] : State->get<SclMap>())
  {
    if (sclState.isWriteData())
    {
      DataInfo *DI = sclState.getDataInfo();
      State = State->set<SclMap>(dataDeclDecl, SclState::getVFenceData(DI));
      stateModified = true;
      llvm::outs() << "vfence VD\n";
    }
    else
    {
      /*
      ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
      if (!ErrNode)
      {
        return;
      }
      BReporter.reportFenceBug(Call, C, ErrNode, C.getBugReporter());
      */
    }
  }

  if (stateModified)
  {
    C.addTransition(State);
  }
}

void OrderingChecker::checkASTDecl(const FunctionDecl *D, AnalysisManager &Mgr,
                                   BugReporter &BR) const
{
  nvmFncInfo.checkFunction(D);
}

void OrderingChecker::checkASTDecl(const DeclaratorDecl *D, AnalysisManager &Mgr,
                                   BugReporter &BR) const
{
  nvmTypeInfo.analyzeMemLabel(D);
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry &registry)
{
  registry.addChecker<clang::ento::nvm::OrderingChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
