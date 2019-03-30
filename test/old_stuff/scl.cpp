#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }

void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

void assign(int* a, int val) { *a = val; }

struct ParentA {
  int a;
};

class ChildB {
  ParentA a;
  int b;
};

struct LogEntry {
  pscl(LogEntry.valid) int data;
  pcheck() int valid;

  LogEntry() {
    data = 0;
    vfence();
    valid = 0;
  }

  void setData(int data_) { data = 1; }

  void setValid(int valid_) { valid = 1; }

  void persistent_code correctUsingPfence() {
    data = 1;
    pfence();
    valid = 1;
  }

  void persistent_code wrongFenceMissing() {
    data = 1;
    valid = 1;
  }
};

void persistent_code correctModelFunction() {
  LogEntry* entry = new LogEntry;
  entry->setData(1);
  vfence();
  entry->setValid(1);
}

void persistent_code wrongWriteToValid() {
  LogEntry* entry = new LogEntry;
  entry->setValid(1);
}