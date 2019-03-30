#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct DclToScl {
  pdcl(DclToScl::valid1) int data;
  pscl(DclToScl::valid2) int valid1;
  pcheck int valid2;

  void persistent_code correct() {
    data = 1;
    clflush(&data);
    pfence();
    valid1 = 1;
    vfence();
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }

  void persistent_code missingVfence() {
    data = 1;
    clflush(&data);
    pfence();
    valid1 = 1;
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }

  void persistent_code missingPfenceData() {
    data = 1;
    clflush(&data);
    valid1 = 1;
    vfence();
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }

  void persistent_code missingPfenceValid() {
    data = 1;
    clflush(&data);
    pfence();
    valid1 = 1;
    vfence();
    valid2 = 1;
    clflush(&valid2);
  }

  void persistent_code missingFlushData() {
    data = 1;
    pfence();
    valid1 = 1;
    vfence();
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }

  void persistent_code missingFlushValid() {
    data = 1;
    clflush(&data);
    pfence();
    valid1 = 1;
    vfence();
    valid2 = 1;
    pfence();
  }
};
