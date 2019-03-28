#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct SimpleCheck {
  pcheck int data;

  void persistent_code correctModelMethod() {
    data = 1;
    clflush(&data);
    pfence();
  }

  void persistent_code wrongWriteOrder() {
    data = 1;
    data = 1;
    pfence();
    clflush(&data);
    clflush(&data);
    data = 1;
    pfence();
    pfence();
    clflush(&data);
  }
};
