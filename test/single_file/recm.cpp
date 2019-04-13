#include "annot.h"
#include <stdio.h>

struct Recm {
  enum {
    MASK = 7,
  };

  sentinelp(Recm::chunk+scl+MASK) int chunk;

  int readData() { return (chunk & ~MASK); }

  int readValid() { return (chunk & MASK); }

  void writeData() { chunk = (chunk & MASK) | 1; }

  void writeValid() { chunk = (chunk & ~MASK) | 1; }

  int analyze_recovery correctCombined() {
    if (readValid()) {
      return readData();
    }
    return 0;
  }

  int analyze_recovery notChecked() { return readData(); }
};
