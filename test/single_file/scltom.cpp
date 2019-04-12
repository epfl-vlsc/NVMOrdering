#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct SclToM {
  enum {
    MASK = 7,
  };
  sentinelp(SclToM::chunk+scl) int data;
  sentinelp(SclToM::chunk+scl+MASK) int chunk;

  void writeData() {
    data = 1;
    chunk = (chunk & MASK) | 1;
  }

  void writeValid() { chunk = (chunk & ~MASK) | 1; }

  void analyze_writes correct() {
    writeData();
    vfence();
    writeValid();
  }

  void analyze_writes correctPfence() {
    writeData();
    pfence();
    writeValid();
  }

  void analyze_writes initWriteDataTwice() {
    writeData();
    writeData();
    vfence();
    writeValid();
  }

  void analyze_writes notFencedData() {
    writeData();
    writeValid();
  }

  void analyze_writes initValid() {
    writeValid();
    writeData();
    vfence();
    writeValid();
  }

  void analyze_writes beforeValidWriteData() {
    writeData();
    vfence();
    writeData();
    writeValid();
  }

  void analyze_writes branch(bool useNvm) {
    writeData();
    if (useNvm) {
      vfence();
      writeValid();
    }
  }
};
