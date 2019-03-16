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
  pdcl(LogEntry.valid) int data;
  pcheck() int valid;

  LogEntry() {
    data = 0;
    clflush(&data);
    pfence();
    valid = 0;
    clflush(&valid);
    pfence();
  }

  void setData(int data_) { data = 1; }

  void setValid(int valid_) { valid = 1; }

  void flushData() { clflush(&data); }

  void flushValid() { clflush(&valid); }

  void persistent_code correctModelMethod() {
    data = 1;
    clflush(&data);
    pfence();
    valid = 1;
    clflush(&valid);
    pfence();
  }

  void persistent_code finalFenceMissing() {
    data = 1;
    clflush(&data);
    pfence();
    valid = 1;
    clflush(&valid);
  }
};

void persistent_code correctModelFunction() {
  LogEntry* entry = new LogEntry;
  entry->setData(1);
  entry->flushData();
  pfence();
  entry->setValid(1);
  entry->flushValid();
  pfence();
}

void persistent_code wrongBranchFunction(bool useNVM) {
  LogEntry* entry = new LogEntry;
  entry->setData(1);
  entry->flushData();
  pfence();
  if (useNVM) {
    entry->setValid(1);
    entry->flushValid();
    pfence();
  }
}

void persistent_code writeToValid() {
  LogEntry* entry = new LogEntry;
  entry->setValid(1);
}

void megaman() {
  int zeman;
  LogEntry entry;
  entry.setData(1);
  entry.flushData();
  pfence();
  entry.setValid(1);
  entry.flushValid();
  pfence();
}