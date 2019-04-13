#include "annot.h"
#include <stdio.h>

struct Recmt {
  enum {
    MASK = 7,
  };

  sentinelp(Recmt::valid+scl+MASK) int chunk;
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

int analyze_recovery readDirect() {
  Recmt* entry = new Recmt;
  return entry->readCD();
  
}

int analyze_recovery correct() {
  Recmt* entry = new Recmt;
  
  if(entry->readValid() && entry->readCC()){
      return entry->readCD();
  }
  return 0;
}

int analyze_recovery readCCFist() {
  Recmt* entry = new Recmt;
  
  if(entry->readCC() && entry->readValid()){
      return entry->readCD();
  }
  return 0;
}

