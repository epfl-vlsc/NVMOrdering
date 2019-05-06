#include "annot.h"

struct Obj {
  int data;
  int valid;

  void analyze_writes correct() {
    data = 1;
    clflushopt(this);
    pfence();
    valid = 1;
  }

  void analyze_writes notFencedData() {
    data = 1;
    clflushopt(this);
    valid = 1;
  }

  void analyze_writes writeDataValid() {
    data = 1;
    valid = 1;
  }

} sentinelp(dcl-Obj::valid);