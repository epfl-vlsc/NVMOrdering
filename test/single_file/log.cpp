#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

//not important if it is log1 or log2
void log_ptr log1(void* ptr){}
void log_ptr log2(void* ptr){}
void log_code log3(){}

struct SimpleLog {
  plog int data1;
  plog int data2;
  int data3;

  void correct() {
    log1(&data1);
    data1 = 1;
    
    log2(&data2);
    data2 = 1;

    data3 = 1;
  }

  void correctLogCode() {
    log3();
    data1 = 1;
    data2 = 1;
    data3 = 1;
  }

  void correctAnyLog() {
    log1(&data1);
    data1 = 1;

    log1(&data2);
    data2 = 1;
    
    log1(&data3);
    data3 = 1;
  }

  void writeBeforeLogCode() {
    data1 = 1;
    log3();
    data2 = 1;
    data3 = 1;
  }

  void noLogData2() {
    log1(&data1);
    data1 = 1;
    
    data2 = 1;
  }

  void noLogData() {
    data1 = 1;
    data2 = 1;
  }

  void noLogData1() {
    data1 = 1;
    
    log2(&data2);
    data2 = 1;
  }
};
