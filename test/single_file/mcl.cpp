#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct FieldDcl {
  sentinelp(dcl-FieldDcl::valid) int data;
  int valid;

  void analyze_writes correct() {
    data = 1;
    clflush(&data);
    pfence();
    valid = 1;
  }

  void analyze_writes correctCircular() {
    valid = 1;
    clflush(&valid);
    pfence();
    data = 1;
    clflush(&data);
    pfence();
    valid = 1;
  }

  void analyze_writes fenceNotFlushedData() {
    data = 1;
    pfence();
    valid = 1;
  }

  void analyze_writes writeFlushedData() {
    data = 1;
    clflush(&data);
    data = 1;
    pfence();
    valid = 1;
  }

  void analyze_writes doubleFlushData() {
    data = 1;
    clflush(&data);
    clflush(&data);
    pfence();
    valid = 1;
  }

  void analyze_writes branch(bool useNvm) {
    data = 1;
    if (useNvm) {
      clflush(&data);
      pfence();
      valid = 1;
    }
  }

  void analyze_writes writeInitValid() {
    valid = 1;
    data = 1;
    clflush(&data);
    pfence();
    valid = 1;
  }

  void analyze_writes writeDataValid() {
    data = 1;
    valid = 1;
    clflush(&data);
    pfence();
    valid = 1;
  }

  void analyze_writes writeFlushValid() {
    data = 1;
    clflush(&data);
    valid = 1;
    pfence();
    valid = 1;
  }
};

struct FieldScl {
  sentinelp(scl-FieldScl::valid) int data;
  int valid;

  void analyze_writes correct() {
    data = 1;
    vfence();
    valid = 1;
  }

  void analyze_writes correctCircular() {
    valid = 1;
    vfence();
    data = 1;
    vfence();
    valid = 1;
  }

  void analyze_writes correctPfence() {
    data = 1;
    pfence();
    valid = 1;
  }

  void analyze_writes notFencedData() {
    data = 1;
    valid = 1;
  }

  void analyze_writes initValid() {
    valid = 1;
    data = 1;
    vfence();
    valid = 1;
  }

  void analyze_writes beforeValidWriteData() {
    data = 1;
    vfence();
    data = 1;
    valid = 1;
  }

  void analyze_writes branch(bool useNvm) {
    data = 1;
    if (useNvm) {
      vfence();
      valid = 1;
    }
  }
};

struct ObjField {
  int data;
  int valid;

  void analyze_writes correct() {
    data = 1;
    vfence();
    valid = 1;
  }

  void analyze_writes notFencedData() {
    data = 1;
    clflush(&data);
    valid = 1;
  }

  void analyze_writes writeDataValid() {
    data = 1;
    valid = 1;
    clflush(&data);
    pfence();
    valid = 1;
  }

} sentinelp(dcl-ObjField::valid);
