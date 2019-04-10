#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct DclToM {
  enum {
    MASK = 7,
  };
  pdcl(DclToM::valid) int data;
  int valid;

  void writeValid() { valid = (valid & ~MASK) | 1; }

  void persistent_code correct() {
    data = 1;
    clflush(&data);
    pfence();
    writeValid();
  }

  void persistent_code initWriteDataTwice() {
    data = 1;
    data = 1;
    clflush(&data);
    pfence();
    writeValid();
  }

  void persistent_code fenceNotFlushedData() {
    data = 1;
    pfence();
    writeValid();
  }

  void persistent_code writeFlushedData() {
    data = 1;
    clflush(&data);
    data = 1;
    pfence();
    writeValid();
  }

  void persistent_code doubleFlushData() {
    data = 1;
    clflush(&data);
    clflush(&data);
    pfence();
    writeValid();
  }

  void persistent_code branch(bool useNvm) {
    data = 1;
    if (useNvm) {
      clflush(&data);
      pfence();
      writeValid();
    }
  }

  void persistent_code writeInitValid() {
    valid = 1;
    data = 1;
    clflush(&data);
    pfence();
    valid = 1;
  }

  void persistent_code writeDataValid() {
    data = 1;
    valid = 1;
    clflush(&data);
    pfence();
    valid = 1;
  }
  
  void persistent_code writeFlushValid() {
    data = 1;
    clflush(&data);
    valid = 1;
    pfence();
    valid = 1;
  }
};
