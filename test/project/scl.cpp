#include "scl.h"

void persistent_code SimpleSCL::correct() {
  data = 1;
  vfence();
  valid = 1;
}

void persistent_code SimpleSCL::initWriteDataTwice() {
  data = 1;
  data = 1;
  vfence();
  valid = 1;
}

void persistent_code SimpleSCL::notFencedData() {
  data = 1;
  valid = 1;
}
