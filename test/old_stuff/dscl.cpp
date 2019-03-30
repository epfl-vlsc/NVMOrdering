#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence() { std::atomic_thread_fence(std::memory_order_release); }

void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

struct LogEntry {
  pdcl(LogEntry.cvalid) int c1data;
  pdcl(LogEntry.cvalid) int c2data;
  pscl(LogEntry.svalid) int sdata;

  pcheck() int cvalid;
  pcheck() int svalid;

  void setData(int data_) {
    c1data = 1;
    c2data = 1;
    sdata = 1;
  }

  void setValid(int valid_) {
    cvalid = 1;
    svalid = 1;
  }

  void flushData() { clflush(&sdata); clflush(&c1data); clflush(&c2data); }

  void flushValid() { clflush(&cvalid); clflush(&svalid); }
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

void persistent_code wrongFenceMissing() {
  LogEntry* entry = new LogEntry;
  entry->setData(1);
  entry->flushData();
  pfence();
  entry->setValid(1);
  entry->flushValid();
}
