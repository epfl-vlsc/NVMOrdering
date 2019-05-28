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
};
