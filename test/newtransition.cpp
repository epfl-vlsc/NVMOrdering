#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }



struct Transition {
  pdcl(Transition.valid1) int data;
  pdcl(SimpleDCL.valid2) int valid1;
  pcheck int valid2;

  void persistent_code correctModelMethod() {
    data = 1;
    clflush(&data);
    pfence();
    valid1 = 1;
    clflush(&valid1);
    pfence();
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }

  void persistent_code wrongWriteOrder() {
    valid2 = 1;
    data = 1;
    clflush(&data);
    pfence();
    valid1 = 1;
    data = 1;
    clflush(&valid1);
    pfence();
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }

  void persistent_code wrongWriteValidFirst() {
    valid2 = 1;
    data = 1;
    clflush(&data);
    pfence();
    valid1 = 1;
    clflush(&valid1);
    pfence();
    valid2 = 1;
    clflush(&valid2);
    pfence();
  }
};
