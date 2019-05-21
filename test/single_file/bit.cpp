#include "annot.h"

struct BitField {
  sentinelp(scl-BitField::valid) int data : 30;
  int valid : 2;

  void analyze_writes noFence() {
    data = 1;
    valid = 1;
  }

  void analyze_writes correct() {
    data = 1;
    vfence();
    valid = 1;
  }
};