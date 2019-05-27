#include "annot.h"

struct Autocl {
  sentinelp(scl-Autocl::valid) int data;
  int valid;

  void correct() {
    data = 1;
    vfence();
    valid = 1;
  }

  void correctCircular() {
    valid = 1;
    vfence();
    data = 1;
    vfence();
    valid = 1;
  }

  void correctPfence() {
    data = 1;
    pfence();
    valid = 1;
  }

  void notFencedData() {
    data = 1;
    valid = 1;
  }

  void initValid() {
    valid = 1;
    data = 1;
    vfence();
    valid = 1;
  }

  void beforeValidWriteData() {
    data = 1;
    vfence();
    data = 1;
    valid = 1;
  }
};