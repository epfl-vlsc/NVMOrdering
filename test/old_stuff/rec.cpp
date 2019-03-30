#include "annot.h"
#include <stdio.h>
#include <utility>

struct LogEntry {
  pdcl(LogEntry.dvalid) int ddata1;
  pdcl(LogEntry.dvalid) int ddata2;
  pcheck() int dvalid;

  pscl(LogEntry.svalid) int sdata;
  pcheck() int svalid;

  int dfix1;
  int dfix2;
  int sfix;

  void recovery_code correctModelMethod() {
    if (dvalid) {
      dfix1 = ddata1;
      dfix2 = ddata2;
    }
    if (svalid) {
      sfix = sdata;
    }
  }

  void recovery_code correctModelMethod2() {
    if (dvalid && svalid) {
      dfix1 = ddata1;
      dfix2 = ddata2;
      sfix = sdata;
    }
  }

  void recovery_code wrongDataAccess() {
    dfix1 = ddata1;
    dfix2 = ddata2;
    sfix = sdata;
  }
};
