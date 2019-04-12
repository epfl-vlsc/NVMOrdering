#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct DclToScl {
  sentinelp(DclToScl::valid1+dcl) int data;
  sentinelp(DclToScl::valid2+scl) int valid1;
  sentinel int valid2;

  void analyze_writes correct() {
    data = 1;
    clflush(&data);
    pfence();
    valid1 = 1;
    vfence();
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }

  void analyze_writes missingVfence() {
    data = 1;
    clflush(&data);
    pfence();
    valid1 = 1;
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }

  void analyze_writes missingPfenceData() {
    data = 1;
    clflush(&data);
    valid1 = 1;
    vfence();
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }

  void analyze_writes missingPfenceValid() {
    data = 1;
    clflush(&data);
    pfence();
    valid1 = 1;
    vfence();
    valid2 = 1;
    clflush(&valid2);
  }

  void analyze_writes missingFlushData() {
    data = 1;
    pfence();
    valid1 = 1;
    vfence();
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }

  void analyze_writes missingFlushValid() {
    data = 1;
    clflush(&data);
    pfence();
    valid1 = 1;
    vfence();
    valid2 = 1;
    pfence();
  }
};
