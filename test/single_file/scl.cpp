#include "annot.h"

struct Scl {
  sentinelp(scl-Scl::valid) int data;
  int valid;

  void analyze_writes correct() {
    data = 1;
    vfence();
    valid = 1;
  }

  void analyze_writes correctCircular() {
    valid = 1;
    vfence();
    data = 1;
    vfence();
    valid = 1;
  }

  void analyze_writes correctPfence() {
    data = 1;
    pfence();
    valid = 1;
  }

  void analyze_writes notFencedData() {
    data = 1;
    valid = 1;
  }

  void analyze_writes initValid() {
    valid = 1;
    data = 1;
    vfence();
    valid = 1;
  }

  void analyze_writes beforeValidWriteData() {
    data = 1;
    vfence();
    data = 1;
    valid = 1;
  }

  void analyze_writes correctBranch(bool useNvm) {
    data = 1;
    if (useNvm) {
      vfence();
      valid = 1;
    }
  }
};