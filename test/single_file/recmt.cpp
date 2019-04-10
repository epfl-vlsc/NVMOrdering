#include "annot.h"
#include <stdio.h>

struct Recmt {
  enum {
    MASK = 7,
  };

  pscl(Recmt::valid) int chunk;
  int valid;

  int readCD() {
    return (chunk & ~MASK);
  }

  int readCC() {
    return (chunk & MASK);
  }

  int readValid(){
    return valid;
  }
};

int recovery_code readDirect() {
  Recmt* entry = new Recmt;
  return entry->readCD();
  
}

int recovery_code correct() {
  Recmt* entry = new Recmt;
  
  if(entry->readValid() && entry->readCC()){
      return entry->readCD();
  }
  return 0;
}

int recovery_code readCCFist() {
  Recmt* entry = new Recmt;
  
  if(entry->readCC() && entry->readValid()){
      return entry->readCD();
  }
  return 0;
}

