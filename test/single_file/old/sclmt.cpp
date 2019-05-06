#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct TransitiveSclm {
  enum {
    MASK = 7,
  };
  sentinelp(TransitiveSclm::valid+scl+MASK) int chunk;
  int valid;

  void writeCD() { chunk = (chunk & MASK) | 1; }

  void writeCV() { chunk = (chunk & ~MASK) | 1; }

  void writeValid() { valid = 1; }

  void analyze_writes correct() {
    writeCD();
    vfence();
    writeCV();
    vfence();
    writeValid();
  }

  void analyze_writes correctPfence() {
    writeCD();
    pfence();
    writeCV();
    pfence();
    writeValid();
  }

  void analyze_writes firstFenceMissing() {
    writeCD();
    writeCV();
    vfence();
    writeValid();
  }

  void analyze_writes secondFenceMissing() {
    writeCD();
    vfence();
    writeCV();
    writeValid();
  }

  void analyze_writes initValid() {
    writeValid();
    writeCD();
    vfence();
    writeCV();
    vfence();
    writeValid();
  }

  void analyze_writes initCV() {
    writeCV();
    writeCD();
    vfence();
    writeCV();
    vfence();
    writeValid();
  }

  void analyze_writes writeValidBeforeCV() {
    writeCD();
    vfence();
    writeValid();
    vfence();
    writeCV();
  }
};