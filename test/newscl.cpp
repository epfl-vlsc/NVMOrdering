#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct SimpleSCL {
  pscl(SimpleSCL::valid) int data;
  int valid;

  void persistent_code correctModelMethod() {
    data = 1;
    vfence();
    valid = 1;
  }

  void persistent_code wrongWriteOrder() {
    data = 1;
    data = 1;
    valid = 1;
    vfence();
    vfence();
    data = 1;
    valid = 1;
    valid = 1;
    
  }

  void persistent_code wrongWriteValidFirst() {
    data = 1;
    vfence();
    valid = 1;
  }
};
