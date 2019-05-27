#include "annot.h"

struct BitField {
  sentinelp(scl-BitField::valid) int data : 30;
  int valid : 2;

  void noFence() {
    data = 1;
    valid = 1;
  }

  void correct() {
    data = 1;
    vfence();
    valid = 1;
  }
};