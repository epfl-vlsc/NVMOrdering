#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct SimpleSclm {
  enum {
    MASK = 7,
  };
  psclm() int data;

  void writeData() { data = (data & MASK) | 1; }

  void writeValid() { data = (data & ~MASK) | 1; }

  void persistent_code correct() {
    writeData();
    vfence();
    writeValid();
  }

  void persistent_code correctPfence() {
    writeData();
    pfence();
    writeValid();
  }

  void persistent_code initWriteDataTwice() {
    writeData();
    writeData();
    vfence();
    writeValid();
  }

  void persistent_code notFencedData() {
    writeData();
    writeValid();
  }

  void persistent_code initValid() {
    writeValid();
    writeData();
    vfence();
    writeValid();
  }

  void persistent_code beforeValidWriteData() {
    writeData();
    vfence();
    writeData();
    writeValid();
  }

  void persistent_code branch(bool useNvm) {
    writeData();
    if (useNvm) {
      vfence();
      writeValid();
    }
  }
};

struct TransitiveSclm {
  enum {
    MASK = 7,
  };
  psclm(TransitiveSclm::valid) int chunk;
  int valid;

  void writeCD() { chunk = (chunk & MASK) | 1; }

  void writeCV() { chunk = (chunk & ~MASK) | 1; }

  void writeValid() { valid = 1; }

  void persistent_code correct() {
    writeCD();
    vfence();
    writeCV();
    vfence();
    writeValid();
  }

  void persistent_code correctPfence() {
    writeCD();
    pfence();
    writeCV();
    pfence();
    writeValid();
  }

  void persistent_code firstFenceMissing() {
    writeCD();
    writeCV();
    vfence();
    writeValid();
  }

  void persistent_code secondFenceMissing() {
    writeCD();
    vfence();
    writeCV();
    writeValid();
  }

  void persistent_code initValid() {
    writeValid();
    writeCD();
    vfence();
    writeCV();
    vfence();
    writeValid();
  }

  void persistent_code initCV() {
    writeCV();
    writeCD();
    vfence();
    writeCV();
    vfence();
    writeValid();
  }

  void persistent_code writeValidBeforeCV() {
    writeCD();
    vfence();
    writeValid();
    vfence();
    writeCV();
  }
};