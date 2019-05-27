#include "annot.h"

struct Dcl {
  sentinelp(dcl-Dcl::valid) int data;
  int valid;

  void correct() {
    data = 1;
    clflushopt(&data);
    pfence();
    valid = 1;
  }

  void correctCircular() {
    valid = 1;
    clflushopt(&valid);
    pfence();
    data = 1;
    clflushopt(&data);
    pfence();
    valid = 1;
  }

  void fenceNotFlushedData() {
    data = 1;
    pfence();
    valid = 1;
  }

  void writeFlushedData() {
    data = 1;
    clflushopt(&data);
    data = 1;
    pfence();
    valid = 1;
  }

  void doubleFlushData() {
    data = 1;
    clflushopt(&data);
    clflushopt(&data);
    pfence();
    valid = 1;
  }

  void correctBranch(bool useNvm) {
    data = 1;
    if (useNvm) {
      clflushopt(&data);
      pfence();
      valid = 1;
    }
  }

  void loop(bool useNvm) {
    data = 1;
    while (true) {
      clflushopt(&data);
      pfence();
      valid = 1;
      clflushopt(&valid);
      pfence();
    }
  }

  void branchNoFence(bool useNvm) {
    data = 1;
    if (useNvm) {
      clflushopt(&data);
      valid = 1;
    }
  }

  void writeInitValid() {
    valid = 1;
    data = 1;
    clflushopt(&data);
    pfence();
    valid = 1;
  }

  void writeDataValid() {
    data = 1;
    valid = 1;
    clflushopt(&data);
    pfence();
    valid = 1;
  }

  void writeFlushValid() {
    data = 1;
    clflushopt(&data);
    valid = 1;
    pfence();
    valid = 1;
  }

  void skip_fnc skip() {
    data = 1;
    clflushopt(&data);
    valid = 1;
    pfence();
    valid = 1;
  }
};
