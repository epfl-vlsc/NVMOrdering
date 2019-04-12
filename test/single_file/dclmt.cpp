#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct TransitiveDclm {
  enum {
    MASK = 7,
  };
  sentinelp(TransitiveDclm::valid+dcl+MASK) int chunk;
  int valid;

  void writeCD() { chunk = (chunk & MASK) | 1; }

  void writeCV() { chunk = (chunk & ~MASK) | 1; }

  void writeValid() { valid = 1; }

  void writeChunk(){
    writeCD();
    vfence();
    writeCV();
  }

  void analyze_writes correct() {
    writeChunk();
    clflush(&chunk);
    pfence();
    valid = 1;
  }

  void analyze_writes initWriteDataTwice() {
    writeChunk();
    writeChunk();
    clflush(&chunk);
    pfence();
    valid = 1;
  }

  void analyze_writes fenceNotFlushedData() {
    writeChunk();
    pfence();
    valid = 1;
  }

  void analyze_writes writeFlushedData() {
    writeChunk();
    clflush(&chunk);
    writeChunk();
    pfence();
    valid = 1;
  }

  void analyze_writes doubleFlushData() {
    writeChunk();
    clflush(&chunk);
    clflush(&chunk);
    pfence();
    valid = 1;
  }

  void analyze_writes branch(bool useNvm) {
    writeChunk();
    if (useNvm) {
      clflush(&chunk);
      pfence();
      valid = 1;
    }
  }

  void analyze_writes writeInitValid() {
    valid = 1;
    writeChunk();
    clflush(&chunk);
    pfence();
    valid = 1;
  }

  void analyze_writes writeDataValid() {
    writeChunk();
    valid = 1;
    clflush(&chunk);
    pfence();
    valid = 1;
  }
  
  void analyze_writes writeFlushValid() {
    writeChunk();
    clflush(&chunk);
    valid = 1;
    pfence();
    valid = 1;
  }
};