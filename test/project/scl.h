#pragma once
#include "nvm.h"

struct SimpleSCL {
  pscl(SimpleSCL::valid) int data;
  int valid;

  void persistent_code correct();

  void persistent_code initWriteDataTwice();

  void persistent_code notFencedData();

  void persistent_code initValid() {
    valid = 1;
    data = 1;
    vfence();
    valid = 1;
  }
};
