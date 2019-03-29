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
    RI.stateChanged = true;
  }else if(CS->isWriteData()){
    //bug:already written data
    RI.reportDataAlreadyWritten();
  }else if(CS->isFlushData()){
    //bug:already written data
    RI.reportDataAlreadyWritten();
  }else if(CS->isPfenceData()){
    //write data
    State = State->set<CheckMap>(D, CheckState::getWriteData());
    RI.stateChanged = true;
  }else {
    llvm::report_fatal_error("not possible");
  }
}

void flushData(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;
  char* D = RI.D;
    
  const CheckState* CS = State->get<CheckMap>(D);

  if(!CS){
    //bug:not written data
    RI.reportDataNotWritten();
  }else if(CS->isWriteData()){
    //flush data
    State = State->set<CheckMap>(D, CheckState::getFlushData());
    RI.stateChanged = true;
  }else if(CS->isFlushData()){
    //bug:already flushed
    RI.reportDataAlreadyFlushed();
  }else if(CS->isPfenceData()){
    //bug: already flushed
    RI.reportDataAlreadyFlushed();
  }else {
    llvm::report_fatal_error("not possible");
  }
}


void pfenceData(ReportInfos& RI) {
  ProgramStateRef& State = RI.State;
  char* D = RI.D;
    
  const CheckState* CS = State->get<CheckMap>(D);

  if(!CS){
    //do nothing
  }else if(CS->isWriteData()){
    //bug:not flushed
    RI.reportDataNotFlushed();
  }else if(CS->isFlushData()){
    //pfence data
    State = State->set<CheckMap>(D, CheckState::getPfenceData());
    RI.stateChanged = true;
  }else if(CS->isPfenceData()){
    //do nothing
  }else {
    llvm::report_fatal_error("not possible");
  }
}


} // namespace clang::ento::nvm::check
