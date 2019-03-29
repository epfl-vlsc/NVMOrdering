#pragma once
#include "Common.h"
#include "SclState.h"

namespace clang::ento::nvm::SclSpace
 {
bool writeData(ProgramStateRef& State, char* D) {
  const SclState* SS = State->get<SclMap>(D);

  if(!SS){
    State = State->set<SclMap>(D, SclState::getWriteData());
  }else if(SS->isWriteData()){
    //bug:already written data
  }else if(SS->isVfenceData()){
    //bug:already written data
  }else if(SS->isWriteCheck()){
    State = State->set<SclMap>(D, SclState::getWriteData());
    return true;
  }else {
    llvm::report_fatal_error("not possible");
  }

  return false;
}

bool vfenceData(ProgramStateRef& State, char* D) {
  const SclState* SS = State->get<SclMap>(D);

  if(!SS){
    return false;
  }else if(SS->isWriteData()){
    State = State->set<SclMap>(D, SclState::getVfenceData());
    return true;
  }else if(SS->isVfenceData()){
    return false;
  }else if(SS->isWriteCheck()){
    return false;
  }else {
    llvm::report_fatal_error("not possible");
  }

  return false;
}


bool writeCheck(ProgramStateRef& State, char* D) {
  const SclState* SS = State->get<SclMap>(D);

  if(!SS){
    //bug:not written data
  }else if(SS->isWriteData()){
    //bug:not fenced data
  }else if(SS->isVfenceData()){
    State = State->set<SclMap>(D, SclState::getWriteCheck());
    return true;
  }else if(SS->isWriteCheck()){
    //bug:not written check
  }else {
    llvm::report_fatal_error("not possible");
  }

  return false;
}


} // namespace clang::ento::nvm::scl
