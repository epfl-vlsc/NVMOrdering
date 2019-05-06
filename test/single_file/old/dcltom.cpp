#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct DclToM {
  enum {
    MASK = 7,
  };
  sentinelp(DclToM::chunk+dcl) int data;
  sentinelp(DclToM::chunk+scl+MASK) int chunk;

  void writeData() {
    data = 1;
    chunk = (chunk & MASK) | 1;
  }

  void writeValid() { chunk = (chunk & ~MASK) | 1; }

  void analyze_writes correct() {
    writeData();
    clflush(&data);
    pfence();
    writeValid();
  }

  void analyze_writes initWriteDataTwice() {
    writeData();
    writeData();
    clflush(&data);
    pfence();
    writeValid();
  }

  void analyze_writes fenceNotFlushedData() {
    writeData();
    pfence();
    writeValid();
  }

  void analyze_writes writeFlushedData() {
    writeData();
    clflush(&data);
    writeData();
    pfence();
    writeValid();
  }

  void analyze_writes doubleFlushData() {
    writeData();
    clflush(&data);
    clflush(&data);
    pfence();
    writeValid();
  }

  void analyze_writes branch(bool useNvm) {
    writeData();
    if (useNvm) {
      clflush(&data);
      pfence();
      writeValid();
    }
  }

  void analyze_writes writeInitValid() {
    writeValid();
    writeData();
    clflush(&data);
    pfence();
    writeValid();
  }

  void analyze_writes writeDataValid() {
    writeData();
    writeValid();
    clflush(&data);
    pfence();
    writeValid();
  }

  void analyze_writes writeFlushValid() {
    data = 1;
    clflush(&data);
    writeValid();
    pfence();
    writeValid();
  }
};
