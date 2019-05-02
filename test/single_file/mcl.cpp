#include "annot.h"

// different cache line placement analysis

struct FieldDcl {
  sentinelp(dcl-FieldDcl::valid) int data;
  int valid;

  void analyze_writes correct() {
    data = 1;
    clflushopt(&data);
    pfence();
    valid = 1;
  }

  void analyze_writes correctCircular() {
    valid = 1;
    clflushopt(&valid);
    pfence();
    data = 1;
    clflushopt(&data);
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
    clflushopt(&data);
    data = 1;
    pfence();
    valid = 1;
  }

  void analyze_writes doubleFlushData() {
    data = 1;
    clflushopt(&data);
    clflushopt(&data);
    pfence();
    valid = 1;
  }

  void analyze_writes branch(bool useNvm) {
    data = 1;
    if (useNvm) {
      clflushopt(&data);
      pfence();
      valid = 1;
    }
  }

  void analyze_writes writeInitValid() {
    valid = 1;
    data = 1;
    clflushopt(&data);
    pfence();
    valid = 1;
  }

  void analyze_writes writeDataValid() {
    data = 1;
    valid = 1;
    clflushopt(&data);
    pfence();
    valid = 1;
  }

  void analyze_writes writeFlushValid() {
    data = 1;
    clflushopt(&data);
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
    clflush(this);
    valid = 1;
  }

  void analyze_writes notFencedData() {
    data = 1;
    clflushopt(this);
    valid = 1;
  }

  void analyze_writes writeDataValid() {
    data = 1;
    valid = 1;
  }

} sentinelp(dcl-ObjField::valid);

struct BitField {
  sentinelp(scl-FieldDcl::valid) int data : 30;
  int valid : 2;

  void analyze_writes correct() {
    data = 1;
    clflush(this);
    valid = 1;
  }
};
