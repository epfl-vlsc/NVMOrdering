#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct SclToDcl {
  sentinelp(SclToDcl::valid1+scl) int data;
  sentinelp(SclToDcl::valid2+dcl) int valid1;
  sentinel int valid2;

  void analyze_writes correct() {
    data = 1;
    vfence();
    valid1 = 1;
    clflush(&valid1);
    pfence();
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }

  void analyze_writes missingVfence() {
    data = 1;
    valid1 = 1;
    clflush(&valid1);
    pfence();
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }

  void analyze_writes missingPfenceData() {
    data = 1;
    vfence();
    valid1 = 1;
    clflush(&valid1);
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }

  void analyze_writes missingPfenceValid() {
    data = 1;
    vfence();
    valid1 = 1;
    clflush(&valid1);
    pfence();
    valid2 = 1;
    clflush(&valid2);
  }

  void analyze_writes missingFlushData() {
    data = 1;
    vfence();
    valid1 = 1;
    pfence();
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }

  void analyze_writes missingFlushValid() {
    data = 1;
    vfence();
    valid1 = 1;
    clflush(&valid1);
    pfence();
    valid2 = 1;
    pfence();
  }
};
