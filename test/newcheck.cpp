#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct SimpleCheck {
  pcheck int data;

  void persistent_code correct() {
    data = 1;
    clflush(&data);
    pfence();
  }

  void persistent_code initWriteDataTwice() {
    data = 1;
    data = 1;
    clflush(&data);
    pfence();
  }

  void persistent_code fenceNotFlushedData() {
    data = 1;
    pfence();
  }

  void persistent_code writeFlushedData() {
    data = 1;
    clflush(&data);
    data = 1;
    pfence();
  }

  void persistent_code doubleFlushData() {
    data = 1;
    clflush(&data);
    clflush(&data);
    pfence();
  }

  void persistent_code branch(bool useNvm) {
    data = 1;
    if (useNvm) {
      clflush(&data);
      pfence();
    }
  }
};
