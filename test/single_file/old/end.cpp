#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

void end_code end(){}

struct End {
  pscl(End::valid) int data;
  int valid;

  void persistent_code correct() {
    data = 1;
    vfence();
    valid = 1;
  }

  void persistent_code correctEnd() {
    data = 1;
    pfence();
    valid = 1;
    end();
  }

  void persistent_code endInitValid() {
    end();
    valid = 1;
    data = 1;
  }

  void persistent_code correctBranch(bool useNvm) {
    data = 1;
    if (useNvm) {
      vfence();
      valid = 1;
    }
  }
};
