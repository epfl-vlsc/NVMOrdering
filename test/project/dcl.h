#pragma once
#include "nvm.h"

struct SimpleDCL {
  pdcl(SimpleDCL::valid) int data;
  int valid;

  void persistent_code correct();

  void persistent_code initWriteDataTwice();

  void persistent_code fenceNotFlushedData();

  void persistent_code doubleFlushData() {
    data = 1;
    clflush(&data);
    clflush(&data);
    pfence();
    valid = 1;
  }
};
