#include "annot.h"
#include <stdio.h>

struct Recm {
  enum {
    MASK = 7,
  };

  pscl() int chunk;

  int readData() {
    return (chunk & ~MASK);
  }

  int readValid() {
    return (chunk & MASK);
  }
};

int recovery_code readDirect() {
  Recm* entry = new Recm;
  return entry->readData();
  
}

int recovery_code correct() {
  Recm* entry = new Recm;
  if(entry->readValid()){
      return entry->readData();
  }
  return 0;
}
