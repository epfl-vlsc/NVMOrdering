#pragma once
#include "Common.h"
#include "CheckState.h"
#include "TransitionInfos.h"

namespace clang::ento::nvm::CheckSpace {

void writeData(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;
  char* D = RI.D;
  const CheckState* CS = State->get<CheckMap>(D);

  if(!CS){
    //write data
    State = State->set<CheckMap>(D, CheckState::getWriteData());
  }else if(CS->isWriteData()){
    //bug:already written data
    
  }else if(CS->isFlushData()){
    //bug:already written data
    
  }else if(CS->isPfenceData()){
    //write data
    State = State->set<CheckMap>(D, CheckState::getWriteData());
    RI.stateChanged = true;
  }else {
    llvm::report_fatal_error("not possible");
  }
}

void flushData(ReportInfos& RI) {
  ProgramStateRef State = RI.State;
  char* D = RI.D;
    
  const CheckState* CS = State->get<CheckMap>(D);

  if(!CS){
    //bug:not written data
  }else if(CS->isWriteData()){
    State = State->set<CheckMap>(D, CheckState::getFlushData());
    RI.stateChanged = true;
  }else if(CS->isFlushData()){
    //bug:already flushed
  }else if(CS->isPfenceData()){
    //bug: already flushed
  }else {
    llvm::report_fatal_error("not possible");
  }
}


void pfenceData(ReportInfos& RI) {
  ProgramStateRef State = RI.State;
  char* D = RI.D;
    
  const CheckState* CS = State->get<CheckMap>(D);

  if(!CS){
    //do nothing
  }else if(CS->isWriteData()){
    //bug:not flushed
  }else if(CS->isFlushData()){
    State = State->set<CheckMap>(D, CheckState::getPfenceData());
    RI.stateChanged = true;
  }else if(CS->isPfenceData()){
    //do nothing
  }else {
    llvm::report_fatal_error("not possible");
  }
}


} // namespace clang::ento::nvm::check
