#include "annot.h"

struct Obj {
  int data;
  int valid;

  void correct() {
    data = 1;
    clflushopt(this);
    pfence();
    valid = 1;
  }

  void notFencedData() {
    data = 1;
    clflushopt(this);
    valid = 1;
  }

  void writeDataValid() {
    data = 1;
    valid = 1;
  }

} sentinelp(dcl-Obj::valid);