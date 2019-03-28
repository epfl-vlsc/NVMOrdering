#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct SimpleMask1 {
  enum {
    MASK = 7,
    NON_MASK = ~MASK,
  };

  pdclm() int data;

  void persistent_code correctModelMethod() {
    data = (data & MASK) | 1;
    vfence();
    data = (data & ~MASK) | 1;
  }

  void persistent_code wrongWriteOrder() {
    data = (data & MASK) | 1;
    data = (data & MASK) | 1;
    data = (data & ~MASK) | 1;
    vfence();
    vfence();
    data = (data & MASK) | 1;
    data = (data & ~MASK) | 1;
  }

  void persistent_code wrongWriteValidFirst() {
    data = (data & ~MASK) | 1;
    data = (data & MASK) | 1;
    vfence();
    data = (data & ~MASK) | 1;
  }
};

struct SimpleMask2 {
  enum {
    MASK = 7,
    NON_MASK = ~MASK,
  };

  pdclm(SimpleMask2.valid) int data;
  int valid;

  void persistent_code correctModelMethod() {
    data = (data & MASK) | 1;
    vfence();
    data = (data & ~MASK) | 1;
    clflush(&data);
    pfence();
    valid = 1;
  }

  void persistent_code wrongWriteOrder() {
    data = (data & MASK) | 1;
    data = (data & MASK) | 1;
    data = (data & ~MASK) | 1;
    vfence();
    vfence();
    data = (data & MASK) | 1;
    data = (data & ~MASK) | 1;
    clflush(&data);
    pfence();
    valid = 1;
  }

  void persistent_code wrongWriteValidFirst() {
    valid = 1;
    data = (data & MASK) | 1;
    vfence();
    data = (data & ~MASK) | 1;
    clflush(&data);
    pfence();
    valid = 1;
  }
};
