#include "dcl.h"

void persistent_code SimpleDCL::correct() {
  data = 1;
  clflush(&data);
  pfence();
  valid = 1;
}

void persistent_code SimpleDCL::initWriteDataTwice() {
  data = 1;
  data = 1;
  clflush(&data);
  pfence();
  valid = 1;
}

void persistent_code SimpleDCL::fenceNotFlushedData() {
  data = 1;
  pfence();
  valid = 1;
}