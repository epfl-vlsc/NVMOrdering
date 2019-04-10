#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct SclToDcl {
  pscl(SclToDcl::valid1) int data;
  pdcl(SclToDcl::valid2) int valid1;
  pcheck int valid2;

  void persistent_code correct() {
    data = 1;
    vfence();
    valid1 = 1;
    clflush(&valid1);
    pfence();
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }

  void persistent_code missingVfence() {
    data = 1;
    valid1 = 1;
    clflush(&valid1);
    pfence();
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }

  void persistent_code missingPfenceData() {
    data = 1;
    vfence();
    valid1 = 1;
    clflush(&valid1);
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }

  void persistent_code missingPfenceValid() {
    data = 1;
    vfence();
    valid1 = 1;
    clflush(&valid1);
    pfence();
    valid2 = 1;
    clflush(&valid2);
  }

  void persistent_code missingFlushData() {
    data = 1;
    vfence();
    valid1 = 1;
    pfence();
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }

  void persistent_code missingFlushValid() {
    data = 1;
    vfence();
    valid1 = 1;
    clflush(&valid1);
    pfence();
    valid2 = 1;
    pfence();
  }
};
