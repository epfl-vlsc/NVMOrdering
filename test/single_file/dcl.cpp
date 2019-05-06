#include "annot.h"

struct Dcl {
  sentinelp(dcl-Dcl::valid) int data;
  int valid;

  void analyze_writes correct() {
    data = 1;
    clflushopt(&data);
    pfence();
    valid = 1;
  }

  void analyze_writes correctCircular() {
    valid = 1;
    clflushopt(&valid);
    pfence();
    data = 1;
    clflushopt(&data);
    pfence();
    valid = 1;
  }

  void analyze_writes fenceNotFlushedData() {
    data = 1;
    pfence();
    valid = 1;
  }

  void analyze_writes writeFlushedData() {
    data = 1;
    clflushopt(&data);
    data = 1;
    pfence();
    valid = 1;
  }

  void analyze_writes doubleFlushData() {
    data = 1;
    clflushopt(&data);
    clflushopt(&data);
    pfence();
    valid = 1;
  }

  void analyze_writes correctBranch(bool useNvm) {
    data = 1;
    if (useNvm) {
      clflushopt(&data);
      pfence();
      valid = 1;
    }
  }

  void analyze_writes loop(bool useNvm) {
    data = 1;
    while (true) {
      clflushopt(&data);
      pfence();
      valid = 1;
      clflushopt(&valid);
      pfence();
    }
  }

  void analyze_writes branchNoFence(bool useNvm) {
    data = 1;
    if (useNvm) {
      clflushopt(&data);
      valid = 1;
    }
  }

  void analyze_writes writeInitValid() {
    valid = 1;
    data = 1;
    clflushopt(&data);
    pfence();
    valid = 1;
  }

  void analyze_writes writeDataValid() {
    data = 1;
    valid = 1;
    clflushopt(&data);
    pfence();
    valid = 1;
  }

  void analyze_writes writeFlushValid() {
    data = 1;
    clflushopt(&data);
    valid = 1;
    pfence();
    valid = 1;
  }
};
