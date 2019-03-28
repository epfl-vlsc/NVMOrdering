#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }



struct SimpleDCL {
  pdcl(SimpleDCL::valid) int data;
  int valid;

  void persistent_code correctModelMethod() {
    data = 1;
    clflush(&data);
    pfence();
    valid = 1;
  }

  void persistent_code wrongWriteOrder() {
    data = 1;
    valid = 1;
    pfence();
    data = 1;
    clflush(&data);
    clflush(&data);
    data = 1;
    valid = 1;
    pfence();
    pfence();
    data = 1;
    clflush(&data);
    valid = 1;
  }

  void persistent_code wrongWriteValidFirst() {
    valid = 1;
    data = 1;
    clflush(&data);
    pfence();
    valid = 1;
  }
};
