#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct SharedValidDcl {
  pdcl(SharedValidDcl::valid) int data1;
  pdcl(SharedValidDcl::valid) int data2;
  int valid;

  void persistent_code correct() {
    data1 = 1;
    clflush(&data1);
    data2 = 1;
    clflush(&data2);
    pfence();
    valid = 1;
  }

  void persistent_code correctManyFence() {
    data1 = 1;
    clflush(&data1);
    pfence();
    data2 = 1;
    clflush(&data2);
    pfence();
    valid = 1;
  }

  void persistent_code flushData1Twice() {
    data1 = 1;
    clflush(&data1);
    data2 = 1;
    clflush(&data1);
    pfence();
    valid = 1;
  }

  void persistent_code fenceBeforeData2() {
    data1 = 1;
    clflush(&data1);
    pfence();
    data2 = 1;
    clflush(&data2);
    valid = 1;
  }

  void persistent_code initWriteValid() {
    valid = 1;
    data1 = 1;
    clflush(&data1);
    data2 = 1;
    clflush(&data2);
    pfence();
    valid = 1;
  }
};

struct SharedValidScl {
  pscl(SharedValidScl::valid) int data1;
  pscl(SharedValidScl::valid) int data2;
  int valid;

  void persistent_code correct() {
    data1 = 1;
    vfence();
    data2 = 1;
    vfence();
    valid = 1;
  }

  void persistent_code correctManyFence() {
    data1 = 1;
    vfence();
    data2 = 1;
    vfence();
    valid = 1;
  }

  void persistent_code fenceBeforeData2() {
    data1 = 1;
    vfence();
    data2 = 1;
    valid = 1;
  }

  void persistent_code initWriteValid() {
    valid = 1;
    data1 = 1;
    data2 = 1;
    vfence();
    valid = 1;
  }
};
