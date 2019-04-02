#include "annot.h"
#include <stdio.h>

struct Recm {
  enum {
    MASK = 7,
  };

  pscl() int chunk;

  int readData() { return (chunk & ~MASK); }

  int readValid() { return (chunk & MASK); }

  void writeData() { chunk = (chunk & MASK) | 1; }

  void writeValid() { chunk = (chunk & ~MASK) | 1; }

  int recovery_code correctCombined() {
    if (readValid()) {
      return readData();
    }
    return 0;
  }

  int recovery_code notChecked() { return readData(); }
};
