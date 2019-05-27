#include "annot.h"

void x() {
  bool z = false;
  for (int i = 5; i < 20; ++i) {
    if (i == 14) {
      i += 5;
    }

    if (z) {
      i += 10;
    }
  }
}
