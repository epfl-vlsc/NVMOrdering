#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct AutoCl {
  sentinelp(AutoCl::valid) int data;
  int valid;

  void analyze_writes correct() {
    data = 1;
    vfence();
    valid = 1;
  }

  void analyze_writes correctPfence() {
    data = 1;
    pfence();
    valid = 1;
  }

  void analyze_writes initWriteDataTwice() {
    data = 1;
    data = 1;
    vfence();
    valid = 1;
  }

  void analyze_writes notFencedData() {
    data = 1;
    valid = 1;
  }

  void analyze_writes initValid() {
    valid = 1;
    data = 1;
    vfence();
    valid = 1;
  }

  void analyze_writes beforeValidWriteData() {
    data = 1;
    vfence();
    data = 1;
    valid = 1;
  }

  void analyze_writes branch(bool useNvm) {
    data = 1;
    if (useNvm) {
      vfence();
      valid = 1;
    }
  }
};
