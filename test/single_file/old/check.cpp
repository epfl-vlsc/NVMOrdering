#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct SimpleSentinel {
  sentinel int data;

  void analyze_writes correct() {
    data = 1;
    clflush(&data);
    pfence();
  }

  void analyze_writes initWriteDataTwice() {
    data = 1;
    data = 1;
    clflush(&data);
    pfence();
  }

  void analyze_writes fenceNotFlushedData() {
    data = 1;
    pfence();
  }

  void analyze_writes writeFlushedData() {
    data = 1;
    clflush(&data);
    data = 1;
    pfence();
  }

  void analyze_writes doubleFlushData() {
    data = 1;
    clflush(&data);
    clflush(&data);
    pfence();
  }

  void analyze_writes branch(bool useNvm) {
    data = 1;
    if (useNvm) {
      clflush(&data);
      pfence();
    }
  }
};
