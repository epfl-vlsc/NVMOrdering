#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

struct LogEntry {
  enum {
    MASK = 7,
  };

  pcheck(MASK) int chunk;

  int readData() {
    return (chunk & MASK);
  }

  int readValid() {
    return (chunk & MASK);
  }
};

int recovery_code wrongReadData() {
  LogEntry* entry = new LogEntry;
  return entry->readData();
  
}

int recovery_code correctModelFunction() {
  LogEntry* entry = new LogEntry;
  if(entry->readValid()){
      return entry->readData();
  }
}
