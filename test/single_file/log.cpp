#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

void log0(){}
void log1(void* ptr){}
void log2(void* ptr){}

struct SimpleLog {
  logging(log0) int data0;
  logging(log1+0) int data1;
  logging(log2+0+SimpleLog::logged) int data2;
  bool logged;

  void analyze_writes correct() {
    log0();
    data0 = 4;

    log1(&data1);
    data1 = 1;
    
    log2(&data2);
    data2 = 1;
  }

  void analyze_writes correctLogCode() {
    log1(&data1);
    data1 = 1;
    
    if(!logged){
      log2(&data2);
    }
    data2 = 1;
  }

  void analyze_writes wrongLog() {
    log2(&data1);
    data1 = 1;
  }

  void analyze_writes forgotLogging() {
    log1(&data1);
    data1 = 1;
    
    data2 = 1;
  }

  void analyze_writes doubleLog() {
    log1(&data1);
    log1(&data1);
    data1 = 1;
    
    data2 = 1;
  }

  void analyze_writes noLogData() {
    data1 = 1;
  }
};
