#pragma once
#include "Common.h"
#include "CheckState.h"

namespace clang::ento::nvm::CheckSpace {

bool writeData(ProgramStateRef& State, char* D) {
  const CheckState* CS = State->get<CheckMap>(D);

  if(!CS){
    State = State->set<CheckMap>(D, CheckState::getWriteData());
  }else if(CS->isWriteData()){
    //bug:already written data
  }else if(CS->isFlushData()){
    //bug:already written data
  }else if(CS->isPfenceData()){
    State = State->set<CheckMap>(D, CheckState::getWriteData());
    return true;
  }else {
    llvm::report_fatal_error("not possible");
  }

  return false;
}

bool flushData(ProgramStateRef& State, char* D) {
  const CheckState* CS = State->get<CheckMap>(D);

  if(!CS){
    //bug:not written data
  }else if(CS->isWriteData()){
    State = State->set<CheckMap>(D, CheckState::getFlushData());
    return true;
  }else if(CS->isFlushData()){
    //bug:already flushed
  }else if(CS->isPfenceData()){
    //bug: already flushed
  }else {
    llvm::report_fatal_error("not possible");
  }

  return false;
}


bool pfenceData(ProgramStateRef& State, char* D) {
  const CheckState* CS = State->get<CheckMap>(D);

  if(!CS){
    return false;
  }else if(CS->isWriteData()){
    //bug:not flushed
  }else if(CS->isFlushData()){
    State = State->set<CheckMap>(D, CheckState::getPfenceData());
    return true;
  }else if(CS->isPfenceData()){
    return false;
  }else {
    llvm::report_fatal_error("not possible");
  }

  return false;
}


} // namespace clang::ento::nvm::check
